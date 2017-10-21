#include "page.h"

int db_fd = -1;
header_page* headerP = NULL;
page* rootP = NULL;

int open_db(char *pathname) {
	int temp;
	db_fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0776);
	headerP = (header_page*)malloc(PAGESIZE);
	rootP = (page*)malloc(PAGESIZE);

	if (db_fd > 0) {
		printf("DB File successfully created\n");
		headerP->rpo = SEEK_SET + PAGESIZE; headerP->num_of_pages = 2;
		rootP->ppo = SEEK_SET; rootP->is_leaf = 0;

		// lseek(db_fd, 0, SEEK_SET);
		pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);
		// lseek(db_fd, sizeof(header_page), SEEK_SET);
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
