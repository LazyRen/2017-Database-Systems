#ifndef __BPT_H__
#define __BPT_H__

//user included header
#include "structure.h"
#include "page.h"

// FUNCTION PROTOTYPES

// Output and utility.
void find_and_print(int64_t key); 
node* find_leaf(off_t *page_loc, int64_t key);
char* find(int64_t key);
int cut(int length);

// Insertion.
int get_left_index(node *parent, int64_t key);
void insert_into_leaf(node *leaf, off_t leaf_loc, int64_t key, char *value);
void insert_into_leaf_after_splitting(node *root, node *leaf, off_t leaf_loc, 
										int64_t key, char *value);
void insert_into_node(node *root, node *parent, off_t parent_loc, 
						int left_index, int64_t key, off_t right_loc);
void insert_into_node_after_splitting(node *root, node *old_node, off_t old_node_loc,
										int left_index, int64_t key, node *right, off_t right_loc);
void insert_into_parent(node *root, node *left, off_t left_loc, int64_t key, 
						  node *right, off_t right_loc);
node* insert_into_new_root(node *left, off_t left_loc, int64_t key, node *right, off_t right_loc);
void start_new_tree(int64_t key, char *value);
int insert(int64_t key, char *value);

#endif /* __BPT_H__*/