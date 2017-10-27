#include "page.h"

int db_fd = -1;

int open_db(char *pathname) {
	int temp;
	db_fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0776);
	headerP = (header_page*)calloc(1, PAGESIZE);

	if (db_fd > 0) {
		printf("DB File successfully created\n");
		headerP->num_pages = 1;

		pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);


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

		return 0;
	}

	printf("Failed to open %s\n", pathname);
	return -1;
}

page* open_page(off_t po)
{
	int temp;
	page* new_page = (page*)malloc(sizeof(PAGESIZE));
	temp = pread(db_fd, new_page, PAGESIZE, SEEK_SET + po);
	if (temp < PAGESIZE) {
		printf("Failed to read page from open_page()\n");
		return NULL;
	}
	return new_page;
}

//get_free_page를 불러오는 page의 npo도 변경해주어야한다!! - 미완
page* get_free_page(off_t ppo, off_t *page_loc, int is_leaf)
{
	int temp;
	off_t fpo = headerP->fpo;
	page* new_page = (page*)malloc(sizeof(PAGESIZE));
	if (fpo == 0) {//no free page avail. Make new page
		new_page->ppo = ppo; new_page->is_leaf = is_leaf;
		pwrite(db_fd, new_page, PAGESIZE, (*page_loc = SEEK_END));
		headerP->num_pages++;
		pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);
	}
	else {//free page is avail.
		temp = pread(db_fd, new_page, PAGESIZE, SEEK_SET + fpo);
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
