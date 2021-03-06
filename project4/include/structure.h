#ifndef __STRUCTURE_H__
#define __STRUCTURE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

//base structure for all pages
#define page_header {\
	off_t ppo;\
	int is_leaf;\
	int num_keys;\
	char ph_reserved[104];\
	off_t expo;\
}
//off_t expo: extra page offset points to
//leaf: right sibling, internal: leftmost child

typedef struct record {
	int64_t key;
	char value[120];
} record;

typedef struct branch_factor {
	int64_t key;
	off_t npo;
} branch_factor;

typedef struct header_page {
	off_t fpo;
	off_t rpo;
	int64_t num_pages;
	char reserved[4072];
} header_page;

//internal, leaf, free page shares same structure using union.
typedef struct page {
	struct page_header;
	union {
		branch_factor entries[248];
		record records[31];
	};
} page;

typedef struct buffer_structure {
	union {
		struct {
			off_t fpo;//free page offset
			off_t rpo;//root page offset
			int64_t num_pages;
			char reserved[4072];
		};
		struct {
			struct page_header;
			union {
				branch_factor entries[248];
				record records[31];
			};
		};
		struct {
			record join_a[16];
			record join_b[16];
		};
	};
	int tid;
	off_t cpo; //current page offset
	bool is_dirty;
	bool refbit;
	int pin_count;
} buffer_structure;

typedef struct buffer_manager {
	int capacity;
	int last_buf;
	//if binary search is in use, this will tell how many buffers are in use for each table
	int table_size[10];
	//buffer location is sorted and will be used for binary search
	struct buf_lookup *buffer_lookup[10];
	struct buffer_structure *buffer_pool;
	//hash_table
	struct buffer_hashframe *hash_table;
} buffer_manager;

typedef struct table_info {
	int fd;
} table_info;

typedef struct buf_lookup {
	int64_t cpo;
	int buf_loc;
} buf_lookup;

typedef struct buffer_hashframe {
	int tid;
	int64_t cpo;
	int buf_loc;
	struct buffer_hashframe *prev;
	struct buffer_hashframe *next;
} buffer_hashframe;

//structures for the queue
typedef struct qnode {
	off_t po;
	struct qnode *next;
} qnode;

typedef struct queue {
	qnode *front;
	qnode *rear;
	int64_t count;
} queue;

//consider node(term used in orignal bpt) structure
//as same as page structure.
typedef struct page node;
#endif