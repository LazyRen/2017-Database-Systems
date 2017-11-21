#ifndef __PAGE_H__
#define __PAGE_H__

#include "structure.h"
#include "bpt.h"

#define PAGESIZE (4096)
#define MAX_TABLE 10
#define toggle_bs false

buffer_manager buf_man;
table_info table[MAX_TABLE];
bool binary_search;

//Utility Functions
void show_buffer_manager(void);
void print_page_info(buffer_structure *cur_page, off_t po, int64_t *total_keys);
void print_tree(int table_id);

int init_db(int num_buf);
int shutdown_db(void);
int open_table(char *pathname);
int close_table(int table_id);
buffer_structure* open_page(int table_id, off_t po);
void write_buffer(buffer_structure *cur_buf);
void drop_pincount(buffer_structure *cur_buf, bool dirty);
void set_dirty(buffer_structure *cur_buf);
buffer_structure* get_free_page(int table_id, off_t ppo, off_t *page_loc, int is_leaf);
void add_free_page(int table_id, off_t page_loc);

//Functions that will be used for print_tree
int bs_buffer(int tid, int64_t cpo);
void insert_buffer(int tid, int64_t cpo, int loc);
void delete_buffer(int tid, int64_t cpo);
void modify_buffer(int tid, int64_t old_cpo, int64_t new_cpo, int bid);
void init_queue(queue *q);
int is_empty(queue *q);
void enqueue(queue *q, off_t data);
off_t dequeue(queue *q);
	
#endif /* __PAGE_H__*/