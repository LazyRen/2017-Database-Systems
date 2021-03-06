#ifndef __PAGE_H__
#define __PAGE_H__

#include "structure.h"
#include "bpt.h"
#define PAGESIZE (4096)

header_page* headerP;
page* rootP;
int db_fd;

//Utility Functions
void print_page_info(page* cur_page, off_t po, int64_t *total_keys);
void print_tree();


int open_db(char *pathname);
page* open_page(off_t po);
page* get_free_page(off_t ppo, off_t *page_loc, int is_leaf);
void add_free_page(off_t page_loc);

//Functions that will be used for print_tree
void init_queue(queue *q);
int is_empty(queue *q);
void enqueue(queue *q, off_t data);
off_t dequeue(queue *q);
	
#endif /* __PAGE_H__*/