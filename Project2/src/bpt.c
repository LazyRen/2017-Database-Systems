#include "bpt.h"

int internal_order = 248;
int leaf_order = 32;
/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
void find_and_print(node *root, int64_t key, bool verbose) {
	char *val = find(root, key, verbose);
	if (val == NULL)
		printf("Record not found under key %lld.\n", key);
	else {
		printf("key %lld, value %s.\n", key, val);
		free(val);
	}
}

node* find_leaf(node *root, off_t *page_loc, int64_t key, bool verbose) {
	int i = 0;
	node *c = root;
	off_t npo;

	if (c == NULL) {
		if (verbose)
			printf("Empty tree.\n");
		return c;
	}
	while (!c->is_leaf) {
		if (verbose) {
			printf("[");
			for (i = 0; i < c->num_keys - 1; i++)
				printf("%lld ", c->entries[i].key);
			printf("%lld] ", c->entries[i].key);
		}
		i = 0;
		while (i < c->num_keys) {
			if (key >= c->entries[i].key) i++;
			else break;
		}
		if (verbose)
			printf("%d ->\n", i);
		npo = c->entries[i].npo;
		if (c != root)
			free(c);
		if (npo == 0) {
			return NULL;
		}
		else
			c = (node *)open_page(c->entries[i].npo);
	}
	if (verbose) {
		printf("Leaf [");
		for (i = 0; i < c->num_keys - 1; i++)
			printf("%lld ", c->entries[i].key);
		printf("%lld] ->\n", c->entries[i].key);
	}
	*page_loc = npo;
	return c;
}

/* Finds and returns the record to which
 * a key refers.
 */
char *find(node *root, int64_t key, bool verbose) {
	int i = 0;
	off_t leaf_loc;
	char *val;

	node *c = find_leaf(root, &leaf_loc, key, verbose);
	if (c == NULL) return NULL;
	for (i = 0; i < c->num_keys; i++)
		if (c->records[i].key == key) break;
	if (i == c->num_keys) {
		free(c);
		return NULL;
	}
	else {
		val = malloc(sizeof(char) * 120);
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

int get_left_index(node * parent, int64_t key) {
	int left_index = 0;

	//indicate to use expo pointer
	if (key < parent->records[0].key)
		return -1;
	while (left_index < parent->num_keys - 1 && 
			parent->records[left_index + 1].key <= key)
		left_index++;
	return left_index;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
void insert_into_leaf(node *leaf, off_t leaf_loc, int64_t key, char * value) {
	int i, insertion_point;

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
	pwrite(db_fd, leaf, PAGESIZE, SEEK_SET + leaf_loc);
	free(leaf);
}

/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
void insert_into_leaf_after_splitting(node *root, node *leaf, off_t leaf_loc, 
										int64_t key, char *value) {

	node *new_leaf;
	int64_t *temp_keys, new_key;
	char **temp_values;
	int insertion_index, split, i, j;
	off_t new_leaf_loc;

	new_leaf = get_free_page(leaf->ppo, &new_leaf_loc, 1);

	temp_keys = malloc(leaf_order * sizeof(int64_t));
	if (temp_keys == NULL) {
		perror("Temporary keys array.");
		exit(EXIT_FAILURE);
	}
	for (int t = 0; t < leaf_order; t++)
		temp_values[t] = (char *)malloc(sizeof(char) * 120);

	temp_values = malloc(leaf_order * sizeof(char *));
	if (temp_values == NULL) {
		perror("Temporary pointers array.");
		exit(EXIT_FAILURE);
	}

	insertion_index = 0;
	while (insertion_index < leaf_order && leaf->records[insertion_index].key < key)
		insertion_index++;

	for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
		if (j == insertion_index) j++;
		temp_keys[j] = leaf->records[i].key;
		strcpy(temp_values[j], leaf->records[i].value);
	}

	temp_keys[insertion_index] = key;
	strcpy(temp_values[insertion_index], value);

	leaf->num_keys = 0;

	split = cut(leaf_order);

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

	for (int t = 0; t < leaf_order; t++)
		free(temp_values[t]);
	free(temp_values);
	free(temp_keys);

	//change right sibling pointer
	new_leaf->expo = leaf->expo;
	leaf->expo = new_leaf_loc;

	for (i = leaf->num_keys; i < leaf_order; i++) {
		leaf->records[i].key = 0;
		memset(leaf->records[i].value, 0, sizeof(char) * 120);
		// strcpy(leaf->records[i].value, '\0');
	}
	for (i = new_leaf->num_keys; i < leaf_order; i++) {
		new_leaf->records[i].key = 0;
		memset(leaf->records[i].value, 0, sizeof(char) * 120);
		// strcpy(leaf->records[i].value, '\0');
	}

	new_leaf->ppo = leaf->ppo;
	new_key = new_leaf->records[0].key;

	pwrite(db_fd, leaf, PAGESIZE, SEEK_SET + leaf_loc);
	pwrite(db_fd, new_leaf, PAGESIZE, SEEK_SET + new_leaf_loc);

	insert_into_parent(root, leaf, leaf_loc, new_key, new_leaf, new_leaf_loc);
}

/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
void insert_into_node(node *root, node *parent, off_t parent_loc, 
                       int left_index, int64_t key, off_t right_loc) {
	int i;

	for (i = parent->num_keys; i > left_index; i--) {
		parent->entries[i + 1].key = parent->entries[i].key;
		parent->entries[i + 1].npo = parent->entries[i].npo;
	}
	parent->entries[left_index + 1].key = key;
	parent->entries[left_index + 1].npo = right_loc;
	parent->num_keys++;
	pwrite(db_fd, parent, PAGESIZE, SEEK_SET + parent_loc);
	return;
}

/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
void insert_into_node_after_splitting(node *root, node *old_node, off_t old_node_loc,
                                      int left_index, int64_t key, node *right, off_t right_loc) {

	int i, j, split;
	node *new_node, *child;
	int64_t *temp_keys, k_prime;
	off_t *temp_npo;
	off_t new_node_loc;

	/* First create a temporary set of keys and pointers
	 * to hold everything in order, including
	 * the new key and pointer, inserted in their
	 * correct places. 
	 * Then create a new node and copy half of the 
	 * keys and pointers to the old node and
	 * the other half to the new.
	 */

	temp_npo = malloc((internal_order + 1) * sizeof(off_t));
	if (temp_npo == NULL) {
		perror("Temporary values array for splitting nodes.");
		exit(EXIT_FAILURE);
	}

	temp_keys = malloc((internal_order + 1) * sizeof(int64_t));
	if (temp_keys == NULL) {
		perror("Temporary keys array for splitting nodes.");
		exit(EXIT_FAILURE);
	}

	for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
		if (j == left_index + 1) j++;
		temp_npo[j] = old_node->entries[i].npo;
	}

	for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
		if (j == left_index + 1) j++;
		temp_keys[j] = old_node->entries[i].key;
	}

	temp_npo[left_index + 1] = right_loc;
	temp_keys[left_index + 1] = key;

	/* Create the new node and copy
	 * half the keys and pointers to the
	 * old and half to the new.
	 */  
	split = cut(internal_order);
	new_node = get_free_page(old_node->ppo, &new_node_loc, 0);
	old_node->num_keys = 0;
	for (i = 0; i < split; i++) {
		old_node->entries[i].npo = temp_npo[i];
		old_node->entries[i].key = temp_keys[i];
		old_node->num_keys++;
	}
	new_node->expo = temp_npo[split];
	k_prime = temp_keys[split + 1];
	for (++i, j = 0; i < internal_order + 1; i++, j++) {
		new_node->entries[j].npo = temp_npo[i];
		new_node->entries[j].key = temp_keys[i];
		new_node->num_keys++;
	}
	free(temp_npo);
	free(temp_keys);

	child = open_page(new_node->expo);
	child->ppo = new_node_loc;
	pwrite(db_fd, child, PAGESIZE, SEEK_SET + new_node->expo);
	free(child);
	for (i = 0; i <= new_node->num_keys; i++) {
		child = open_page(new_node->entries[i].npo);
		child->ppo = new_node_loc;
		pwrite(db_fd, child, PAGESIZE, SEEK_SET + new_node->entries[i].npo);
		free(child);
	}

	/* Insert a new key into the parent of the two
	 * nodes resulting from the split, with
	 * the old node to the left and the new to the right.
	 */

	insert_into_parent(root, old_node, old_node_loc, k_prime, new_node, new_node_loc);
}

/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
void insert_into_parent(node *root, node *left, off_t left_loc, int64_t key, 
						  node *right ,off_t right_loc) {

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

	if (parent->num_keys < internal_order)
		insert_into_node(root, parent, left->ppo,left_index, key, right_loc);

	/* Harder case:  split a node in order 
	 * to preserve the B+ tree properties.
	 */

	else 
		insert_into_node_after_splitting(root, parent, left->ppo, left_index, key, right, right_loc);

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
	off_t root_loc;
	node *root = get_free_page(SEEK_SET, &root_loc, 0);
	root->entries[0].key = key;
	root->expo = left_loc;
	root->entries[0].npo = right_loc;
	root->num_keys++;
	left->ppo = root_loc;
	right->ppo = root_loc;
	pwrite(db_fd, root, PAGESIZE, SEEK_SET + root_loc);
	pwrite(db_fd, left, PAGESIZE, SEEK_SET + left_loc);
	pwrite(db_fd, right, PAGESIZE, SEEK_SET + right_loc);
	return root;
}

/* First insertion:
 * start a new tree.
 */
node* start_new_tree(int64_t key, char *value) {
	off_t root_loc;

	node *root = get_free_page(SEEK_SET, &root_loc, 1);
	rootP = root;

	//cpy records
	root->records[0].key = key;
	strcpy(root->records[0].value, value);
	root->num_keys++;
	pwrite(db_fd, root, PAGESIZE, SEEK_SET + root_loc);

	//update header page
	headerP->rpo = root_loc;
	pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);
	return root;
}

/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
int insert(node *root, int64_t key, char *value) {
	off_t leaf_loc;
	node *leaf;
	/* The current implementation ignores
	 * duplicates.
	 */

	if (find(root, key, false) != NULL) {
		printf("key already exists\n");
		return -1;
	}

	if (strlen(value) >= 120) {
		printf("%s is too long for value\n", value);
		return -1;
	}

	/* Case: the tree does not exist yet.
	 * Start a new tree.
	 */

	if (root == NULL) {
		rootP = start_new_tree(key, value);
		return 0;
	}


	/* Case: the tree already exists.
	 * (Rest of function body.)
	 */

	leaf = find_leaf(root, &leaf_loc, key, false);

	/* Case: leaf has room for key and pointer.
	 */

	if (leaf->num_keys < leaf_order) {
		insert_into_leaf(leaf, leaf_loc, key, value);
		return 0;
	}


	/* Case:  leaf must be split.
	 */

	// insert_into_leaf_after_splitting(root, leaf, &leaf_loc, key, value);
	free(leaf);
	return 0;
}