#include "bpt.h"

int internal_order = 249;//# of keys in internal page. actual pointer is +1.
int leaf_order = 32;
bool verbose = true; //set it true to see how things are working.
bool debugging = true;//more info. for debugging

/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
void find_and_print(int64_t key) {
	char *val = find(key);
	if (val == NULL)
		printf("Record not found under key: %lld\n", key);
	else {
		printf("key: %lld, value: %s\n", key, val);
		free(val);
	}
}

//find a leaf page containing key.
//must free leaf page after use
node* find_leaf(off_t *page_loc, int64_t key) {
	int i = 0;
	node *c = open_page(headerP->rpo);
	off_t npo = headerP->rpo;

	if (c == NULL) {
		if (verbose)
			printf("Empty tree.\n");
		return c;
	}
	while (!c->is_leaf) {
		if (verbose) {
			printf("Internal page at: %lld\n", npo);
			printf("# of keys: %d\n", c->num_keys);
			if (debugging) {
				printf("[");
				for (i = 0; i < c->num_keys - 1; i++)
					printf("%lld ", c->entries[i].key);
				printf("%lld] ", c->entries[i].key);
			}
		}
		i = -1;
		while (i < c->num_keys - 1) {
			if (key >= c->entries[i + 1].key) i++;
			else break;
		}
		if (verbose)
			printf("%d ->\n", i);
		if (i == -1)
			npo = c->expo;
		else
			npo = c->entries[i].npo;
		if (npo == 0) {
			return NULL;
		}
		else
			c = (node *)open_page(c->entries[i].npo);
	}
	*page_loc = npo;
	if (verbose) {
		printf("leaf page at: %lld\n", *page_loc);
		printf("# of keys: %d\n", c->num_keys);
		if (debugging) {
			printf("Leaf [");
			for (i = 0; i < c->num_keys - 1; i++)
				printf("%lld ", c->records[i].key);
			printf("%lld] ->\n", c->records[i].key);
		}
	}
	
	return c;
}

/* Finds and returns the record to which
 * a key refers.
 */
//val must be freed after use.
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

	//indicate to use expo pointer
	if (key < parent->records[0].key)
		return -1;
	while (left_index < parent->num_keys - 1 && 
			parent->records[left_index + 1].key <= key)
		left_index++;
	if (verbose)
		printf("left index: %d\n", left_index);
	return left_index;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
void insert_into_leaf(node *leaf, off_t leaf_loc, int64_t key, char *value) {
	if (debugging)
		printf("insert_into_leaf(%lld: %lld)\n", key, leaf_loc);
	int i, insertion_point, temp;

	insertion_point = 0;
	while (insertion_point < leaf->num_keys && leaf->records[insertion_point].key < key)
		insertion_point++;
	if (debugging)
		printf("Insertion Point: %d\n", insertion_point);
	for (i = leaf->num_keys; i > insertion_point; i--) {
		leaf->records[i].key = leaf->records[i - 1].key;
		strcpy(leaf->records[i].value, leaf->records[i - 1].value);
	}
	leaf->records[insertion_point].key = key;
	strcpy(leaf->records[insertion_point].value, value);
	leaf->num_keys++;
	temp = pwrite(db_fd, leaf, PAGESIZE, leaf_loc);
	if (temp < PAGESIZE) {
		printf("%lld\n", leaf_loc);
		printf("Failed to write leaf page\n");
		printf("%s\n", strerror(errno));
	}
	if(verbose) {
		node * testP;
		testP = open_page(leaf_loc);
		printf("# of keys: %d\n", testP->num_keys);
		for (int t = 0; t < testP->num_keys; t++) {
			printf("[%lld:%s] ", testP->records[t].key, testP->records[t].value);
		}
		printf("\n");
		free(testP);
	}

	free(leaf);
}

/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
void insert_into_leaf_after_splitting(node *leaf, off_t leaf_loc, 
										int64_t key, char *value) {
	if (debugging)
		printf("insert_into_leaf_after_splitting(%lld, %lld)\n", key, leaf_loc);
	node *new_leaf;
	int64_t temp_keys[33], new_key;
	char temp_values[33][120];
	int insertion_index, split, i, j;
	off_t new_leaf_loc;
	new_leaf = get_free_page(leaf->ppo, &new_leaf_loc, 1);
	if (debugging)
		printf("for splitting leaf: new_leaf at %lld\n", new_leaf_loc);

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
	if (debugging) {
		printf("\ntotal leaf key\n");
		for (int t = 0; t < leaf_order; t++)
			printf("[%lld:%s] ", temp_keys[t], temp_values[t]);
		printf("\n");
	}
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
	
	for (i = leaf->num_keys; i < leaf_order; i++) {
		leaf->records[i].key = 0;
		memset(leaf->records[i].value, '\0', 120);
	}
	for (i = new_leaf->num_keys; i < leaf_order; i++) {
		new_leaf->records[i].key = 0;
		memset(new_leaf->records[i].value, '\0', 120);
	}

	if (debugging) {
		printf("leaf\n");
		for (int t = 0; t < leaf->num_keys; t++)
			printf("[%lld:%s]", leaf->records[t].key, leaf->records[t].value);
		printf("\nnew_leaf\n");
		for (int t = 0; t < new_leaf->num_keys; t++)
			printf("[%lld:%s]", new_leaf->records[t].key, new_leaf->records[t].value);
		printf("\n");
	}

	new_leaf->ppo = leaf->ppo;
	new_key = new_leaf->records[0].key;

	pwrite(db_fd, leaf, PAGESIZE, leaf_loc);
	pwrite(db_fd, new_leaf, PAGESIZE, new_leaf_loc);

	insert_into_parent(leaf, leaf_loc, new_key, new_leaf, new_leaf_loc);
}

/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
void insert_into_node(node *parent, off_t parent_loc, 
                       int left_index, int64_t key, off_t right_loc) {
	int i;

	for (i = parent->num_keys; i > left_index; i--) {
		parent->entries[i + 1].key = parent->entries[i].key;
		parent->entries[i + 1].npo = parent->entries[i].npo;
	}
	parent->entries[left_index + 1].key = key;
	parent->entries[left_index + 1].npo = right_loc;
	parent->num_keys++;
	pwrite(db_fd, parent, PAGESIZE, parent_loc);
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
	split = cut(internal_order - 1);
	new_node = get_free_page(old_node->ppo, &new_node_loc, 0);
	old_node->num_keys = 0;
	for (i = 0; i < split; i++) {
		old_node->entries[i].npo = temp_npo[i];
		old_node->entries[i].key = temp_keys[i];
		old_node->num_keys++;
	}
	new_node->expo = temp_npo[split];
	k_prime = temp_keys[split + 1];
	for (i = split + 1, j = 0; i < internal_order; i++, j++) {
		new_node->entries[j].npo = temp_npo[i];
		new_node->entries[j].key = temp_keys[i];
		new_node->num_keys++;
	}

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
}

/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
void insert_into_parent(node *left, off_t left_loc, int64_t key, 
						  node *right ,off_t right_loc) {
	if (debugging) {
		printf("insert_into_parent called\n");
		printf("left_loc: %lld\nkey: %lld\nright_loc: %lld\n", left_loc, key, right_loc);
	}
	int left_index;
	node *parent;

	/* Case: new root. */

	if (left->ppo == SEEK_SET) {
		rootP = insert_into_new_root(left, left_loc, key, right, right_loc);
		return;
	}

	parent = open_page(left->ppo);
	if (parent == NULL) {
		perror("Failed to find parent - insert_into_parent()");
		exit(EXIT_FAILURE);
	}

	/* Case: leaf or node. (Remainder of
	 * function body.)  
	 */

	/* Find the parent's pointer to the left 
	 * node.
	 */

	left_index = get_left_index(parent, left->entries[0].key);
	//if return value is '-1' offset is located at addr 120-128(expo)

	/* Simple case: the new key fits into the node. 
	 */

	if (parent->num_keys < internal_order - 1)
		insert_into_node(parent, left->ppo,left_index, key, right_loc);

	/* Harder case:  split a node in order 
	 * to preserve the B+ tree properties.
	 */

	else 
		insert_into_node_after_splitting(parent, left->ppo, left_index, key, right, right_loc);

	free(left);
	free(right);
	free(parent);
}

/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
node* insert_into_new_root(node *left, off_t left_loc, int64_t key, 
						   node *right, off_t right_loc) {
	if (debugging)
		printf("insert_into_new_root called\n");
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

	//cpy records
	rootP->records[0].key = key;
	strcpy(rootP->records[0].value, value);
	rootP->num_keys++;
	pwrite(db_fd, rootP, PAGESIZE, root_loc);

	//update header page
	headerP->rpo = root_loc;
	pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);

	if (verbose)
		printf("new root saved at : %lld\n", root_loc);
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
		printf("key already exists\n");
		printf("value: %s\n", val);
		free(val);
		return -1;
	}

	/* Case: the tree already exists.
	 * (Rest of function body.)
	 */

	leaf = find_leaf(&leaf_loc, key);
	if (verbose) {
		printf("found leaf at : %lld\n", leaf_loc);
	}

	/* Case: leaf has room for key and pointer.
	 */

	if (leaf->num_keys < leaf_order - 1) {
		insert_into_leaf(leaf, leaf_loc, key, value);
		return 0;
	}


	/* Case:  leaf must be split.
	 */

	insert_into_leaf_after_splitting(leaf, leaf_loc, key, value);
	free(leaf);
	return 0;
}