#ifndef __PAGE_H__
#define __PAGE_H__

#include "bpt.h"
#include "structure.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define PAGESIZE (4096)


//user defined functions
int open_db(char *pathname);

//user defined global variables
extern int db_fd;
extern header_page* headerP;
extern page* rootP;
	
#endif /* __PAGE_H__*/
