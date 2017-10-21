bool verbose_output = false;

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
node * find_leaf(node * root, int64_t key, bool verbose) {
	int i = 0;
	node * c = root;
	if (c == NULL) {
		if (verbose) 
			printf("Empty tree.\n");
		return c;
	}
	while (!c->is_leaf) {
		if (verbose) {
			printf("[");
			for (i = 0; i < c->num_keys - 1; i++)
				printf("%d ", c->keys[i]);
			printf("%d] ", c->keys[i]);
		}
		i = 0;
		while (i < c->num_of_keys) {
			if (key >= c->entries[i].key) i++;
			else break;
		}
		if (verbose)
			printf("%d ->\n", i);
		c = (node *)c->pointers[i];
	}
	if (verbose) {
		printf("Leaf [");
		for (i = 0; i < c->num_keys - 1; i++)
			printf("%d ", c->keys[i]);
		printf("%d] ->\n", c->keys[i]);
	}
	return c;
}

/* Finds and returns the record to which
 * a key refers.
 */
record * find(node * root, int64_t key, bool verbose) {
	int i = 0;
	node * c = find_leaf(root, key, verbose);
	if (c == NULL) return NULL;
	for (i = 0; i < c->num_of_keys; i++)
		if (c->records[i].key == key) break;
	if (i == c->num_of_keys) 
		return NULL;
	else
		return &(c->records[i]);
}

/* Creates a new record to hold the value
 * to which a key refers.
 */
record * make_record(char* value) {
	record * new_record = (record *)malloc(sizeof(record));
	if (new_record == NULL) {
		perror("Record creation.");
		exit(EXIT_FAILURE);
	}
	else {
		if(strlen(value) > 120) {
			perror("Record creation - value length too long");
		exit(EXIT_FAILURE);
		}
		strcpy(record->value, value)
	}
	return new_record;
}

node * make_node(void) {
	off_t written;
	node * new_node;
	new_node = malloc(sizeof(node));
	if (new_node == NULL) {
		perror("Node creation.");
		exit(EXIT_FAILURE);
	}
	new_node->is_leaf = 0;
	new_node->num_of_keys = 0;
	return new_node;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
node * make_leaf(void) {
	node * leaf = make_node();
	leaf->is_leaf = 1;
	return leaf;
}

/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
node * insert_into_node(node * root, node * n, int left_index, int key, node * right) {
	int i;

	for (i = n->num_keys; i > left_index; i--) {
		n->pointers[i + 1] = n->pointers[i];
		n->keys[i] = n->keys[i - 1];
	}
	n->pointers[left_index + 1] = right;
	n->keys[left_index] = key;
	n->num_keys++;
	return root;
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
node * insert_into_node_after_splitting(node * root, node * old_node, int left_index, int key, node * right) {

	int i, j, split, k_prime;
	node * new_node, * child;
	int * temp_keys;
	node ** temp_pointers;

	/* First create a temporary set of keys and pointers
	 * to hold everything in order, including
	 * the new key and pointer, inserted in their
	 * correct places. 
	 * Then create a new node and copy half of the 
	 * keys and pointers to the old node and
	 * the other half to the new.
	 */

	temp_pointers = malloc( (order + 1) * sizeof(node *) );
	if (temp_pointers == NULL) {
		perror("Temporary pointers array for splitting nodes.");
		exit(EXIT_FAILURE);
	}
	temp_keys = malloc( order * sizeof(int) );
	if (temp_keys == NULL) {
		perror("Temporary keys array for splitting nodes.");
		exit(EXIT_FAILURE);
	}

	for (i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {
		if (j == left_index + 1) j++;
		temp_pointers[j] = old_node->pointers[i];
	}

	for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
		if (j == left_index) j++;
		temp_keys[j] = old_node->keys[i];
	}

	temp_pointers[left_index + 1] = right;
	temp_keys[left_index] = key;

	/* Create the new node and copy
	 * half the keys and pointers to the
	 * old and half to the new.
	 */  
	split = cut(order);
	new_node = make_node();
	old_node->num_keys = 0;
	for (i = 0; i < split - 1; i++) {
		old_node->pointers[i] = temp_pointers[i];
		old_node->keys[i] = temp_keys[i];
		old_node->num_keys++;
	}
	old_node->pointers[i] = temp_pointers[i];
	k_prime = temp_keys[split - 1];
	for (++i, j = 0; i < order; i++, j++) {
		new_node->pointers[j] = temp_pointers[i];
		new_node->keys[j] = temp_keys[i];
		new_node->num_keys++;
	}
	new_node->pointers[j] = temp_pointers[i];
	free(temp_pointers);
	free(temp_keys);
	new_node->parent = old_node->parent;
	for (i = 0; i <= new_node->num_keys; i++) {
		child = new_node->pointers[i];
		child->parent = new_node;
	}

	/* Insert a new key into the parent of the two
	 * nodes resulting from the split, with
	 * the old node to the left and the new to the right.
	 */

	return insert_into_parent(root, old_node, k_prime, new_node);
}

node * start_new_tree(int key, record * pointer) {

	node * root = make_leaf();
	root->keys[0] = key;
	root->pointers[0] = pointer;
	root->pointers[order - 1] = NULL;
	root->parent = NULL;
	root->num_keys++;
	return root;
}

node * insert(node * root, int key, char* value) {

	record * pointer;
	node * leaf;

	/* The current implementation ignores
	 * duplicates.
	 */

	if (find(root, key, false) != NULL)
		return root;

	/* Create a new record for the
	 * value.
	 */
	pointer = make_record(value);


	/* Case: the tree does not exist yet.
	 * Start a new tree.
	 */

	if (root == NULL) 
		return start_new_tree(key, pointer);


	/* Case: the tree already exists.
	 * (Rest of function body.)
	 */

	leaf = find_leaf(root, key, false);

	/* Case: leaf has room for key and pointer.
	 */

	if (leaf->num_keys < order - 1) {
		leaf = insert_into_leaf(leaf, key, pointer);
		return root;
	}


	/* Case:  leaf must be split.
	 */

	return insert_into_leaf_after_splitting(root, leaf, key, pointer);
}