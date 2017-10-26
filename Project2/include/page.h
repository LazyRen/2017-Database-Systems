#ifndef __PAGE_H__
#define __PAGE_H__

#include "structure.h"
#include "bpt.h"
#define PAGESIZE (4096)


//user defined functions
int open_db(char *pathname);
page* open_page(off_t po);
page* get_free_page(off_t *page_loc);
//user defined global variables
extern int db_fd;
extern header_page* headerP;
extern page* rootP;
	
#endif /* __PAGE_H__*/