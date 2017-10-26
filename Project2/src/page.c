#include "page.h"

int db_fd = -1;
header_page* headerP = NULL;
page* rootP = NULL;

int open_db(char *pathname) {
	int temp;
	db_fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0776);
	headerP = (header_page*)calloc(1, PAGESIZE);
	rootP = (page*)calloc(1, PAGESIZE);

	if (db_fd > 0) {
		printf("DB File successfully created\n");
		headerP->rpo = SEEK_SET + PAGESIZE; headerP->num_of_pages = 2;
		rootP->ppo = SEEK_SET; rootP->is_leaf = 1;

		pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);
		pwrite(db_fd, rootP, PAGESIZE, SEEK_SET + PAGESIZE);


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
		temp = pread(db_fd, rootP, PAGESIZE, SEEK_SET + headerP->rpo);
		if (temp < PAGESIZE) {
			printf("Failed to read root_page \n");
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
page* get_free_page(off_t *page_loc)
{
	int temp;
	off_t fpo = headerP->fpo;

	if (fpo == 0)
		return NULL;
	page* new_page = (page*)malloc(sizeof(PAGESIZE));
	temp = pread(db_fd, new_page, PAGESIZE, (*page_loc = SEEK_SET + fpo));
	if (temp < PAGESIZE) {
		printf("Failed to read page from get_free_page()\n");
		return NULL;
	}
	fpo = new_page->ppo;
	new_page->ppo = 0;
	//need to be more specific
	pwrite(db_fd, new_page, PAGESIZE, SEEK_SET + headerP->fpo);
	headerP->fpo = fpo;
	//need to be more specific
	pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);
	return new_page;
}
