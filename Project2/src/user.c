#ifndef __USER_H__
#define __USER_H__
#ifndef __BPT_H__
// #include "bpt.h"
#include </Users/LazyRen/Google 드라이브/2 - 2/데이터베이스시스템/project/Project2/include/bpt.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define PAGESIZE 4096

//structures for 4 kinds of page
typedef struct header_page {
	off_t fpo;
	off_t rpo;
	int64_t num_of_pages;
	char reserved[4072];
} header_page;

typedef struct free_page {
	off_t nfpo;
	char reserved[4086];
} free_page;

typedef struct internal_page {
	off_t ppo;
	int is_leaf;
	int num_of_keys;
	char ph_reserved[104];
	off_t krpo; //not sure when to use YET
	branch_factor entries[248];

} internal_page;

typedef struct leaf_page {
	off_t ppo;
	int is_leaf;
	int num_of_keys;
	char ph_reserved[104];
	off_t rspo;
	record records[31];
} leaf_page;


//user defined global variables
int db_fd = -1;
header_page* headerP;
internal_page* rootP;

int open_db(char *pathname) {
	int temp;
	db_fd = open(pathname, O_RDWR | O_CREAT | O_EXCL, 0776);
	headerP = (header_page*)malloc(sizeof(header_page));
	rootP = (internal_page*)malloc(sizeof(internal_page));

	if (db_fd > 0) {
		printf("DB File successfully created\n");
		headerP->rpo = SEEK_SET + PAGESIZE; headerP->num_of_pages = 2;
		rootP->ppo = SEEK_SET; rootP->is_leaf = 0;

		lseek(db_fd, 0, SEEK_SET);
		write(db_fd, headerP, sizeof(header_page));
		lseek(db_fd, sizeof(header_page), SEEK_SET);
		printf("%d\n", temp);
		write(db_fd, rootP, sizeof(internal_page));


		return 0;
	}

	db_fd = open(pathname, O_RDWR);
	if (db_fd > 0) {
		printf("Existing DB File has been successfully opened\n");
		lseek(db_fd, 0, SEEK_SET);
		temp = read(db_fd, headerP, PAGESIZE);
		if (temp < PAGESIZE) {
			printf("Failed to read header_page\n");
			exit(EXIT_FAILURE);
		}
		lseek(db_fd, headerP->rpo, SEEK_SET);
		temp = read(db_fd, rootP, PAGESIZE);
		if (temp < PAGESIZE) {
			printf("Failed to read root_page \n");
			exit(EXIT_FAILURE);
		}

		return 0;
	}

	printf("Failed to open %s\n", pathname);
	return -1;
}

int main()
{
	int a;
	a = open_db("/Users/LazyRen/Documents/Programming/TEST/text");
	if (a != 0)
		printf("failed\n");
}

#endif /* __USER_H__*/