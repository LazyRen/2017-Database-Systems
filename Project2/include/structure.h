#ifndef __STRUCTURE_H__
#define __STRUCTURE_H__
//structures for types of page
#define page_header {\
	off_t ppo;\
	int is_leaf;\
	int num_of_keys;\
	char ph_reserved[104];\
	off_t rspo;\
}

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
	int64_t num_of_pages;
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


typedef struct page node;
#endif