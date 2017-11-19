#ifndef __PAGE_H__
#define __PAGE_H__

#include "structure.h"
#include "bpt.h"
#define PAGESIZE (4096)
#define MAX_TABLE 10

buffer_manager buf_man;
table_info table[MAX_TABLE];

//Utility Functions
void print_page_info(page* cur_page, off_t po, int64_t *total_keys);
void print_tree();


int init_db(int num_buf);
int open_table(char *pathname);
buffer_structure* open_page(int table_id, off_t po);
void write_buffer(buffer_structure *cur_buf);
void drop_pincount(buffer_structure *cur_buf, bool dirty);
void set_dirty(buffer_structure *cur_buf);
page* get_free_page(off_t ppo, off_t *page_loc, int is_leaf);
void add_free_page(off_t page_loc);

//Functions that will be used for print_tree
void init_queue(queue *q);
int is_empty(queue *q);
void enqueue(queue *q, off_t data);
off_t dequeue(queue *q);
	
#endif /* __PAGE_H__*/