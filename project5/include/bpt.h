#ifndef __BPT_H__
#define __BPT_H__

//user included header
#include "page.h"
#include "log_recovery.h"

// FUNCTION PROTOTYPES

// Output and utility.
void find_and_print(int table_id, int64_t key);
buffer_structure* find_leaf(buffer_structure* headerP, int table_id, off_t *page_loc, int64_t key);
char* find(int table_id, int64_t key);
int cut(int length);

// Insertion.
int get_left_index(buffer_structure *parent, int64_t key);
void insert_into_leaf(buffer_structure *leaf, off_t leaf_loc, int64_t key, char *value);
void insert_into_leaf_after_splitting(buffer_structure *leaf, off_t leaf_loc,
										int64_t key, char *value);
void insert_into_node(buffer_structure *parent, off_t parent_loc,
						int left_index, int64_t key, buffer_structure *right, off_t right_loc);
void insert_into_node_after_splitting(buffer_structure *old_node, off_t old_node_loc,
										int left_index, int64_t key, buffer_structure *right, off_t right_loc);
void insert_into_parent(buffer_structure *left, off_t left_loc, int64_t key,
						  buffer_structure *right, off_t right_loc);
void insert_into_new_root(buffer_structure *left, off_t left_loc, int64_t key,
                           buffer_structure *right, off_t right_loc);
void start_new_tree(buffer_structure *headerP, int table_id, int64_t key, char *value);
int insert(int table_id, int64_t key, char *value);

// Deletion
int get_neighbor_index(buffer_structure *parent_page, off_t child_loc, off_t *neighbor_loc);
void remove_entry_from_node(buffer_structure *cur_page, int64_t key);
void adjust_root(buffer_structure *root);
void coalesce_nodes(buffer_structure *cur_page, buffer_structure *neighbor, int neighbor_index, int64_t k_prime);
void redistribute_nodes(buffer_structure *cur_page, buffer_structure *neighbor, int neighbor_index, int64_t k_prime, int k_prime_index);
void delete_entry(buffer_structure *cur_page, int64_t key);
int delete(int table_id, int64_t key);

int update(int table_id, int64_t key, char *value);
#endif /* __BPT_H__*/
