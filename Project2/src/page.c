#include "page.h"

int db_fd = -1;

int open_db(char *pathname) {
	int temp;
	off_t test;
	db_fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0776);
	headerP = (header_page*)calloc(1, PAGESIZE);

	if (db_fd > 0) {
		printf("DB File successfully created\n");
		headerP->num_pages = 1;
		temp = pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);
		if (temp < PAGESIZE) {
			printf("Failed to write header_page\n");
			exit(EXIT_FAILURE);
		}

		return 0;
	}

	db_fd = open(pathname, O_RDWR | O_SYNC);
	if (db_fd > 0) {
		printf("Existing DB File has been successfully opened\n");
		temp = pread(db_fd, headerP, PAGESIZE, SEEK_SET);
		if (temp < PAGESIZE) {
			printf("Failed to read header_page\n");
			exit(EXIT_FAILURE);
		}
		if (headerP->rpo != 0) {
			rootP = (page *)malloc(PAGESIZE);
			temp = pread(db_fd, rootP, PAGESIZE, headerP->rpo);
			if (temp < PAGESIZE) {
				printf("%lld\n", headerP->rpo);
				printf("Failed to read root_page\n");
				printf("%s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		else {
			printf("DB is empty\n");
		}
		return 0;
	}

	printf("Failed to open %s\n", pathname);
	return -1;
}

//read and open the page from the disk.
//must free page after use.
page* open_page(off_t po)
{
	int temp;
	page* new_page = (page*)malloc(sizeof(PAGESIZE));
	temp = pread(db_fd, new_page, PAGESIZE, po);
	if (temp < PAGESIZE) {
		printf("Failed to read page from open_page()\n");
		return NULL;
	}
	return new_page;
}

//get_free_page is the only function that actually writes new page
//into the DB file. If there is free page available, it returns free page,
//if not, create new page and writes it and return the new page.
page* get_free_page(off_t ppo, off_t *page_loc, int is_leaf)
{
	int temp = 0;
	off_t fpo = headerP->fpo;
	page* new_page = (page*)calloc(1, sizeof(PAGESIZE));
	if (fpo == 0) {//no free page avail. Make new page
		new_page->ppo = ppo; new_page->is_leaf = is_leaf;
		*page_loc = lseek(db_fd, 0, SEEK_END);
		temp = pwrite(db_fd, new_page, PAGESIZE, PAGESIZE * headerP->num_pages);
		if (temp < PAGESIZE) {
			printf("Failed to write from get_free_page()\n");
			exit(EXIT_FAILURE);
		}
		
		headerP->num_pages++;
		pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);
	}
	else {//free page is avail.
		temp = pread(db_fd, new_page, PAGESIZE, fpo);
		*page_loc = fpo; //disk loc. of read page
		if (temp < PAGESIZE) {
			printf("Failed to read page from get_free_page()\n");
			return NULL;
		}
		fpo = new_page->ppo;
		new_page->ppo = ppo; //set ppo to calling page
		new_page->is_leaf = is_leaf;
		pwrite(db_fd, new_page, PAGESIZE, SEEK_SET + headerP->fpo);
		headerP->fpo = fpo;
		pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);
	}
	return new_page;
}
