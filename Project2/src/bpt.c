#include "bpt.h"

//Any page will have at most order - 1 keys
int internal_order = 249;
int leaf_order = 32;

/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
void find_and_print(int64_t key) {
	char *val = find(key);
	if (val == NULL) {
		printf("Record not found under key: %"PRId64"\n", key);
	}
	else {
		printf("key: %"PRId64", value: %s\n", key, val);
		free(val);
	}
}

//find a leaf page containing key.
//must free leaf page after use
node* find_leaf(off_t *page_loc, int64_t key) {
	int i = 0;
	off_t npo = headerP->rpo;
	if (npo == SEEK_SET) {
		return NULL;
	}
	node *c = open_page(npo);
	while (!c->is_leaf) {
		i = -1;
		while (i < c->num_keys - 1) {
			if (key >= c->entries[i + 1].key) i++;
			else break;
		}
		if (i == -1)
			npo = c->expo;
		else
			npo = c->entries[i].npo;
		if (npo == 0) {
			return NULL;
		}
		else {
			free(c);
			c = open_page(npo);
		}
	}
	*page_loc = npo;
	
	return c;
}

/* Finds and returns the record to which
 * a key refers.
 */
//return val must be freed after use.
char* find(int64_t key) {
	int i = 0;
	off_t leaf_loc;
	char *val;
	if (rootP == NULL) {
		if (headerP->rpo == 0)
			return NULL;
		else
			rootP = open_page(headerP->rpo);
	}

	node *c = find_leaf(&leaf_loc, key);
	if (c == NULL) return NULL;
	for (i = 0; i < c->num_keys; i++)
		if (c->records[i].key == key) break;
	if (i == c->num_keys) {
		free(c);
		return NULL;
	}
	else {
		val = calloc(1, sizeof(char) * 120);
		strcpy(val, c->records[i].value);
		free(c);
		return val;
	}
}

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int cut(int length) {
	if (length % 2 == 0)
		return length/2;
	else
		return length/2 + 1;
}

int get_left_index(node *parent, int64_t key) {
	int left_index = 0;
	if (parent->is_leaf) {
		//indicate to use expo pointer
		if (key < parent->records[0].key)
			return -1;
		while (left_index < parent->num_keys - 1 && 
				parent->records[left_index + 1].key <= key)
			left_index++;
	}
	else {
		if (key < parent->entries[0].key)
			return -1;
		while (left_index < parent->num_keys - 1 && 
				parent->entries[left_index + 1].key <= key)
			left_index++;
	}
	return left_index;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
void insert_into_leaf(node *leaf, off_t leaf_loc, int64_t key, char *value) {
	int i, insertion_point, temp;

	insertion_point = 0;
	while (insertion_point < leaf->num_keys && leaf->records[insertion_point].key < key)
		insertion_point++;
	for (i = leaf->num_keys; i > insertion_point; i--) {
		leaf->records[i].key = leaf->records[i - 1].key;
		strcpy(leaf->records[i].value, leaf->records[i - 1].value);
	}
	leaf->records[insertion_point].key = key;
	strcpy(leaf->records[insertion_point].value, value);
	leaf->num_keys++;
	temp = pwrite(db_fd, leaf, PAGESIZE, leaf_loc);
	if (temp < PAGESIZE) {
		printf("Failed to write leaf page\n");
		printf("%s\n", strerror(errno));
	}
}

/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
void insert_into_leaf_after_splitting(node *leaf, off_t leaf_loc, 
										int64_t key, char *value) {
	node *new_leaf;
	int64_t temp_keys[leaf_order + 1], new_key;
	char temp_values[leaf_order + 1][120];
	int insertion_index, split, i, j;
	off_t new_leaf_loc;
	new_leaf = get_free_page(leaf->ppo, &new_leaf_loc, 1);

	insertion_index = 0;
	while (insertion_index < leaf_order - 1 && leaf->records[insertion_index].key < key)
		insertion_index++;

	for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
		if (j == insertion_index) j++;
		temp_keys[j] = leaf->records[i].key;
		strcpy(temp_values[j], leaf->records[i].value);
	}

	temp_keys[insertion_index] = key;
	strcpy(temp_values[insertion_index], value);
	leaf->num_keys = 0;

	split = cut(leaf_order - 1);

	for (i = 0; i < split; i++) {
		leaf->records[i].key = temp_keys[i];
		strcpy(leaf->records[i].value, temp_values[i]);
		leaf->num_keys++;
	}

	for (i = split, j = 0; i < leaf_order; i++, j++) {
		new_leaf->records[j].key = temp_keys[i];
		strcpy(new_leaf->records[j].value, temp_values[i]);
		new_leaf->num_keys++;
	}

	//change right sibling pointer
	new_leaf->expo = leaf->expo;
	leaf->expo = new_leaf_loc;

	for (i = leaf->num_keys; i < leaf_order - 1; i++) {
		leaf->records[i].key = 0;
		memset(leaf->records[i].value, '\0', sizeof(char) * 120);
	}
	for (i = new_leaf->num_keys; i < leaf_order - 1; i++) {
		new_leaf->records[i].key = 0;
		memset(new_leaf->records[i].value, '\0', sizeof(char) * 120);
	}

	new_leaf->ppo = leaf->ppo;
	new_key = new_leaf->records[0].key;

	pwrite(db_fd, leaf, PAGESIZE, leaf_loc);
	pwrite(db_fd, new_leaf, PAGESIZE, new_leaf_loc);

	insert_into_parent(leaf, leaf_loc, new_key, new_leaf, new_leaf_loc);

	free(new_leaf);
}

/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
void insert_into_node(node *parent, off_t parent_loc, 
                       int left_index, int64_t key, node *right, off_t right_loc) {
	int i;

	for (i = parent->num_keys - 1; i > left_index; i--) {
		parent->entries[i + 1].key = parent->entries[i].key;
		parent->entries[i + 1].npo = parent->entries[i].npo;
	}
	parent->entries[left_index + 1].key = key;
	parent->entries[left_index + 1].npo = right_loc;
	parent->num_keys++;
	pwrite(db_fd, parent, PAGESIZE, parent_loc);
	right->ppo = parent_loc;
	pwrite(db_fd, right, PAGESIZE, right_loc);

	return;
}

/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
void insert_into_node_after_splitting(node *old_node, off_t old_node_loc,
                                      int left_index, int64_t key, node *right, off_t right_loc) {
	int i, j, split;
	node *new_node, *child;
	int64_t temp_keys[internal_order + 1], k_prime;
	off_t temp_npo[internal_order + 1];
	off_t new_node_loc;

	/* First create a temporary set of keys and pointers
	 * to hold everything in order, including
	 * the new key and pointer, inserted in their
	 * correct places. 
	 * Then create a new node and copy half of the 
	 * keys and pointers to the old node and
	 * the other half to the new.
	 */

	for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
		if (j == left_index + 1) j++;
		temp_npo[j] = old_node->entries[i].npo;
	}

	for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
		if (j == left_index + 1) j++;
		temp_keys[j] = old_node->entries[i].key;
	}

	temp_keys[left_index + 1] = key;
	temp_npo[left_index + 1] = right_loc;

	/* Create the new node and copy
	 * half the keys and pointers to the
	 * old and half to the new.
	 */  
	split = cut(internal_order) - 1;
	new_node = get_free_page(old_node->ppo, &new_node_loc, 0);
	old_node->num_keys = 0;
	for (i = 0; i < split; i++) {
		old_node->entries[i].npo = temp_npo[i];
		old_node->entries[i].key = temp_keys[i];
		old_node->num_keys++;
	}
	new_node->expo = temp_npo[split];
	k_prime = temp_keys[split];
	new_node->num_keys = 0;
	for (i = split + 1, j = 0; i < internal_order; i++, j++) {
		new_node->entries[j].npo = temp_npo[i];
		new_node->entries[j].key = temp_keys[i];
		new_node->num_keys++;
	}
	pwrite(db_fd, old_node, PAGESIZE, old_node_loc);
	pwrite(db_fd, new_node, PAGESIZE, new_node_loc);

	child = open_page(new_node->expo);
	child->ppo = new_node_loc;
	pwrite(db_fd, child, PAGESIZE, new_node->expo);
	free(child);
	for (i = 0; i <= new_node->num_keys; i++) {
		child = open_page(new_node->entries[i].npo);
		child->ppo = new_node_loc;
		pwrite(db_fd, child, PAGESIZE, new_node->entries[i].npo);
		free(child);
	}

	/* Insert a new key into the parent of the two
	 * nodes resulting from the split, with
	 * the old node to the left and the new to the right.
	 */

	insert_into_parent(old_node, old_node_loc, k_prime, new_node, new_node_loc);

	free(new_node);
}

/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
void insert_into_parent(node *left, off_t left_loc, int64_t key, 
						  node *right ,off_t right_loc) {
	int left_index;
	node *parent;

	/* Case: new root. */

	if (left->ppo == SEEK_SET) {
		free(rootP);
		rootP = insert_into_new_root(left, left_loc, key, right, right_loc);
		return;
	}

	parent = open_page(left->ppo);
	if (parent == NULL) {
		perror("Failed to find parent - insert_into_parent()");
		exit(EXIT_FAILURE);
	}


	/* Find the parent's pointer to the left 
	 * node.
	 */

	left_index = get_left_index(parent, left->entries[0].key);
	//if return value is '-1' offset is located at addr 120-128(expo)

	/* Simple case: the new key fits into the node. 
	 */
	if (parent->num_keys < internal_order - 1)
		insert_into_node(parent, left->ppo, left_index, key, right, right_loc);

	/* Harder case:  split a node in order 
	 * to preserve the B+ tree properties.
	 */
	else 
		insert_into_node_after_splitting(parent, left->ppo, left_index, key, right, right_loc);

	free(parent);
}

/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
node* insert_into_new_root(node *left, off_t left_loc, int64_t key, 
						   node *right, off_t right_loc) {
	off_t root_loc;
	node *root = get_free_page(SEEK_SET, &root_loc, 0);
	
	root->ppo = SEEK_SET;
	root->entries[0].key = key;
	root->expo = left_loc;
	root->entries[0].npo = right_loc;
	root->num_keys++;
	left->ppo = root_loc;
	right->ppo = root_loc;
	pwrite(db_fd, root, PAGESIZE, root_loc);
	pwrite(db_fd, left, PAGESIZE, left_loc);
	pwrite(db_fd, right, PAGESIZE, right_loc);

	headerP->rpo = root_loc;
	pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);

	return root;
}

/* First insertion:
 * start a new tree.
 */
void start_new_tree(int64_t key, char *value) {
	off_t root_loc;

	rootP = get_free_page(SEEK_SET, &root_loc, 1);

	//copy records
	rootP->records[0].key = key;
	strcpy(rootP->records[0].value, value);
	rootP->num_keys++;
	pwrite(db_fd, rootP, PAGESIZE, root_loc);

	//update header page
	headerP->rpo = root_loc;
	pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);

	return;
}

/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
//creates Root Page if there is none read from DB.
//rootP stays in memory while program runs.
int insert(int64_t key, char *value) {
	off_t leaf_loc;
	node *leaf;
	char *val;

	if (strlen(value) >= 120) {
		printf("%s is too long for value\n", value);
		return -1;
	}

	/* Case: the tree does not exist yet.
	 * Start a new tree.
	 */
	if (rootP == NULL) {
		if (headerP->rpo == 0) {
			start_new_tree(key, value);
			return 0;
		}
		else
			rootP = open_page(headerP->rpo);
	}


	/* The current implementation ignores
	 * duplicates.
	 */

	if ((val = find(key)) != NULL) {
		printf("%"PRId64" key already exists\n", key);
		printf("value: %s\n", val);
		free(val);
		return -1;
	}

	/* Case: the tree already exists.
	 * (Rest of function body.)
	 */

	leaf = find_leaf(&leaf_loc, key);

	/* Case: leaf has room for key and pointer.
	 */

	if (leaf->num_keys < leaf_order - 1)
		insert_into_leaf(leaf, leaf_loc, key, value);

	/* Case:  leaf must be split.
	 */
	else
		insert_into_leaf_after_splitting(leaf, leaf_loc, key, value);

	free(leaf);
	return 0;
}

// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -2
 * if the node is at index [0], returns -1
 */
int get_neighbor_index(node *child_page, off_t child_loc, off_t *neighbor_loc) {

	int i;
	node *tmp = open_page(child_loc);
	// node *parent_page = open_page(child_page->ppo);
	node *parent_page = open_page(tmp->ppo);
	/* Return the index of the key to the left
	 * of the pointer in the parent pointing
	 * to child_page.  
	 * If child_page is the leftmost child, this means
	 * return -2. If it is located at index[0], return -1. 
	 */
	if (parent_page->expo == child_loc) {
		*neighbor_loc = parent_page->entries[0].npo;
		free(parent_page);
		return -2;
	}
	for (i = 0; i < parent_page->num_keys; i++)
		if (parent_page->entries[i].npo == child_loc) {
			*neighbor_loc = (i == 0 ? parent_page->expo : parent_page->entries[i - 1].npo);
			free(parent_page);
			return i - 1;
		}

	// Error state.
	printf("Search for nonexistent neighbor in parent.\n");
	exit(EXIT_FAILURE);
}

void adjust_root(node *root) {
	node *new_root;
	off_t old_root_loc;

	/* Case: nonempty root.
	 * Key and pointer have already been deleted,
	 * so nothing to be done.
	 */

	if (root->num_keys > 0) {
		return;
	}

	old_root_loc = headerP->rpo;
	/* Case: empty root. 
	 */

	// If it has a child, promote 
	// the first (only) child
	// as the new root.

	if (!root->is_leaf) {
		headerP->rpo = root->expo;
		pwrite(db_fd, headerP, PAGESIZE, 0);

		new_root = open_page(headerP->rpo);
		new_root->ppo = SEEK_SET;

		free(rootP);
		add_free_page(old_root_loc);
		rootP = new_root;
		pwrite(db_fd, new_root, PAGESIZE, headerP->rpo);
	}

	// If it is a leaf (has no children),
	// then the whole tree is empty.

	else {
		headerP->rpo = 0;
		pwrite(db_fd, headerP, PAGESIZE, 0);

		add_free_page(old_root_loc);
		free(rootP);
		rootP = NULL;
	}


	return;
}

node* remove_entry_from_node(node *cur_page, off_t page_loc, int64_t key) {
	int i;

	// Remove the key and shift other keys accordingly.
	if (cur_page->is_leaf) {
		i = 0;
		while (cur_page->records[i].key != key)
			i++;
		for (++i; i < cur_page->num_keys; i++) {
			cur_page->records[i - 1].key = cur_page->records[i].key;
			strcpy(cur_page->records[i - 1].value, cur_page->records[i].value);
		}
	}
	else {
		i = 0;
		while (cur_page->entries[i].key != key)
			i++;
		for (++i; i < cur_page->num_keys; i++) {
			cur_page->entries[i - 1].key = cur_page->entries[i].key;
			cur_page->entries[i - 1].npo = cur_page->entries[i].npo;
		}
	}

	// One key fewer.
	cur_page->num_keys--;

	// Set the other pointers to NULL for tidiness.
	// A leaf uses the last pointer to point to the next leaf.
	if (cur_page->is_leaf) {
		for (i = cur_page->num_keys; i < leaf_order - 1; i++) {
			cur_page->records[i].key = 0;
			memset(cur_page->records[i].value, '\0', sizeof(char) * 120);
		}
	}
	else {
		for (i = cur_page->num_keys; i < internal_order - 1; i++) {
			cur_page->entries[i].key = 0;
			cur_page->entries[i].npo = 0;
		}
	}

	pwrite(db_fd, cur_page, PAGESIZE, page_loc);
	return cur_page;
}

/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */
void coalesce_nodes(node *cur_page, off_t page_loc, node *neighbor, off_t neighbor_loc, int neighbor_index, int64_t k_prime) {
	int i, j, neighbor_insertion_index, n_end;
	node *tmp, *parent;
	off_t tmp_loc;
	/* Swap neighbor with node if node is on the
	 * extreme left and neighbor is to its right.
	 */
	parent = open_page(cur_page->ppo);
	if (neighbor_index == -2) {
		tmp = cur_page;
		cur_page = neighbor;
		neighbor = tmp;
		tmp_loc = page_loc;
		page_loc = neighbor_loc;
		neighbor_loc = tmp_loc;
	}

	/* Starting point in the neighbor for copying
	 * keys and pointers from n.
	 * Recall that n and neighbor have swapped places
	 * in the special case of n being a leftmost child.
	 */

	neighbor_insertion_index = neighbor->num_keys;

	/* Case:  nonleaf node.
	 * Append k_prime and the following pointer.
	 * Append all pointers and keys from the neighbor.
	 */

	if (!cur_page->is_leaf) {
		/* Append k_prime.
		 */
		neighbor->entries[neighbor_insertion_index].key = k_prime;
		neighbor->entries[neighbor_insertion_index].npo = cur_page->expo;
		neighbor->num_keys++;

		n_end = cur_page->num_keys;

		for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
			neighbor->entries[i].key = cur_page->entries[j].key;
			neighbor->entries[i].npo = cur_page->entries[j].npo;
			neighbor->num_keys++;
			cur_page->num_keys--;
		}

		/* All children must now point up to the same parent.
		 */
		for (i = neighbor_insertion_index; i < neighbor->num_keys; i++) {
			tmp = open_page(neighbor->entries[i].npo);
			tmp->ppo = neighbor_loc;
			pwrite(db_fd, tmp, PAGESIZE, neighbor->entries[i].npo);
			free(tmp);
		}
	}

	/* In a leaf, append the keys and pointers of
	 * n to the neighbor.
	 * Set the neighbor's last pointer to point to
	 * what had been n's right neighbor.
	 */

	else {
		for (i = neighbor_insertion_index, j = 0; j < cur_page->num_keys; i++, j++) {
			neighbor->records[i].key = cur_page->records[j].key;
			strcpy(neighbor->records[i].value, cur_page->records[j].value);
			neighbor->num_keys++;
		}
		neighbor->expo = cur_page->expo;
	}

	pwrite(db_fd, neighbor, PAGESIZE, neighbor_loc);
	add_free_page(page_loc);
	delete_entry(parent, cur_page->ppo, k_prime);
	free(parent);
	return;
}

/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 */
void redistribute_nodes(node *cur_page, off_t page_loc, node *neighbor, off_t neighbor_loc, int neighbor_index, int64_t k_prime, int k_prime_index) {
	int i;
	node *tmp, *parent;
	parent = open_page(cur_page->ppo);
	/* Case: n has a neighbor to the left. 
	 * Pull the neighbor's last key-pointer pair over
	 * from the neighbor's right end to n's left end.
	 */

	if (neighbor_index != -2) {
		if (cur_page->is_leaf) {
			for (i = cur_page->num_keys; i > 0; i--) {
				cur_page->records[i].key = cur_page->records[i - 1].key;
				strcpy(cur_page->records[i].value, cur_page->records[i - 1].value);
			}
			cur_page->records[0].key = neighbor->records[neighbor->num_keys - 1].key;
			strcpy(cur_page->records[0].value, neighbor->records[neighbor->num_keys - 1].value);
			parent->entries[k_prime_index].key = cur_page->records[0].key;

			neighbor->records[neighbor->num_keys - 1].key = 0;
			memset(neighbor->records[neighbor->num_keys - 1].value, '\0', 120);
		}
		else {
			for (i = cur_page->num_keys; i > 0; i--) {
				cur_page->entries[i].key = cur_page->entries[i - 1].key;
				cur_page->entries[i].npo = cur_page->entries[i - 1].npo;
			}
			cur_page->entries[0].key = k_prime;
			cur_page->entries[0].npo = cur_page->expo;
			cur_page->expo = neighbor->entries[neighbor->num_keys - 1].npo;

			tmp = open_page(cur_page->expo);
			tmp->ppo = page_loc;
			pwrite(db_fd, tmp, PAGESIZE, cur_page->expo);
			parent->entries[k_prime_index].key = neighbor->entries[neighbor->num_keys - 1].key;
			free(tmp);
			neighbor->entries[neighbor->num_keys - 1].key = 0;
			neighbor->entries[neighbor->num_keys - 1].npo = 0;
		}
	}

	/* Case: n is the leftmost child.
	 * Take a key-pointer pair from the neighbor to the right.
	 * Move the neighbor's leftmost key-pointer pair
	 * to n's rightmost position.
	 */

	else {  
		if (cur_page->is_leaf) {
			cur_page->records[cur_page->num_keys].key = neighbor->records[0].key;
			strcpy(cur_page->records[cur_page->num_keys].value, neighbor->records[0].value);

			for (i = 0; i < neighbor->num_keys - 1; i++) {
				neighbor->records[i].key = neighbor->records[i + 1].key;
				strcpy(neighbor->records[i].value, neighbor->records[i + 1].value);
			}
			parent->entries[k_prime_index].key = neighbor->records[0].key;
		}
		else {
			cur_page->entries[cur_page->num_keys].key = k_prime;
			cur_page->entries[cur_page->num_keys].npo = neighbor->expo;
			parent->entries[k_prime_index].key = neighbor->entries[0].key;
			tmp = open_page(cur_page->entries[cur_page->num_keys].npo);
			tmp->ppo = page_loc;
			pwrite(db_fd, tmp, PAGESIZE, cur_page->entries[cur_page->num_keys].npo);
			free(tmp);

			neighbor->expo = neighbor->entries[0].npo;
			for (i = 0; i < neighbor->num_keys - 1; i++) {
				neighbor->entries[i].key = neighbor->entries[i + 1].key;
				neighbor->entries[i].npo = neighbor->entries[i + 1].npo;
			}
		}
	}

	/* n now has one more key and one more pointer;
	 * the neighbor has one fewer of each.
	 */

	cur_page->num_keys++;
	neighbor->num_keys--;

	pwrite(db_fd, neighbor, PAGESIZE, neighbor_loc);
	pwrite(db_fd, cur_page, PAGESIZE, page_loc);
	pwrite(db_fd, parent, PAGESIZE, cur_page->ppo);
	free(parent);
	return;
}

void delete_entry(node *cur_page, off_t page_loc, int64_t key) {
	node *neighbor, *parent;
	off_t neighbor_loc;
	int64_t k_prime;
	int k_prime_index;
	int min_keys;
	int neighbor_index;
	int capacity;

	// Remove key and pointer from node.

	cur_page = remove_entry_from_node(cur_page, page_loc, key);

	/* Case:  deletion from the root. 
	 */

	if (cur_page->ppo == SEEK_SET) {
		adjust_root(cur_page);
		return;
	}


	/* Case:  deletion from a node below the root.
	 * (Rest of function body.)
	 */

	/* Determine minimum allowable size of node,
	 * to be preserved after deletion.
	 */
	//leaf: 16 internal: 124
	min_keys = cur_page->is_leaf ? cut(leaf_order - 1) : cut(internal_order) - 1;

	/* Case:  node stays at or above minimum.
	 * (The simple case.)
	 */

	if (cur_page->num_keys >= min_keys)
		return;

	/* Case:  node falls below minimum.
	 * Either coalescence or redistribution
	 * is needed.
	 */

	/* Find the appropriate neighbor node with which
	 * to coalesce.
	 * Also find the key (k_prime) in the parent
	 * between the pointer to node n and the pointer
	 * to the neighbor.
	 */
	//-2: leftmost, -1: neighbor is in expo
	neighbor_index = get_neighbor_index(cur_page, page_loc, &neighbor_loc);
	parent = open_page(cur_page->ppo);
	neighbor = open_page(neighbor_loc);
	if (neighbor_index == -2) {
		k_prime_index = 0;
		k_prime = parent->entries[k_prime_index].key;
	}
	else {
		k_prime_index = neighbor_index + 1;
		k_prime = parent->entries[k_prime_index].key;
	}
	capacity = cur_page->is_leaf ? leaf_order : internal_order - 1;

	/* Coalescence. */

	if (neighbor->num_keys + cur_page->num_keys < capacity)
		coalesce_nodes(cur_page, page_loc, neighbor, neighbor_loc, neighbor_index, k_prime);

	/* Redistribution. */

	else
		redistribute_nodes(cur_page, page_loc, neighbor, neighbor_loc, neighbor_index, k_prime, k_prime_index);

	free(parent);
	free(neighbor);
	return;
}

int delete(int64_t key) {
	node *key_leaf;
	char *val;
	off_t leaf_loc;

	if ((val = find(key)) == NULL) {
		printf("key does not exists. Failed to delete\n");
		return -1;
	}

	key_leaf = find_leaf(&leaf_loc, key);
	delete_entry(key_leaf, leaf_loc, key);
	free(val);
	free(key_leaf);
	return 0;
}