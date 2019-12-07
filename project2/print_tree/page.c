#include "page.h"

int db_fd = -1;

int open_db(char *pathname) {
	int temp;
	printf("open db at %s\n", pathname);
	db_fd = open(pathname, O_RDWR);
	headerP = (header_page*)calloc(1, PAGESIZE);

	if (db_fd > 0) {
		printf("Existing DB File has been successfully opened\n");
		temp = pread(db_fd, headerP, PAGESIZE, SEEK_SET);
		if (temp < PAGESIZE) {
			printf("Failed to read header_page\n");
			exit(EXIT_FAILURE);
		}
		printf("fpo:%"PRId64"\nrpo:%"PRId64"\n# of pages:%"PRId64"\n", headerP->fpo, headerP->rpo, headerP->num_pages);
		if (headerP->rpo != 0) {
			rootP = (page *)calloc(1, PAGESIZE);
			temp = pread(db_fd, rootP, PAGESIZE, headerP->rpo);
			if (temp < PAGESIZE) {
				printf("%"PRId64"\n", headerP->rpo);
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
	page* new_page = (page*)calloc(1, PAGESIZE);
	temp = pread(db_fd, new_page, PAGESIZE, po);
	if (temp < PAGESIZE) {
		printf("Failed to read page from open_page()\n");
		return NULL;
	}
	return new_page;
}
void print_page_info(page* cur_page, off_t po, int64_t *total_keys)
{
	printf("%s page at %"PRId64" - %"PRId64"\n", cur_page->is_leaf ? "leaf" : "internal", cur_page->ppo/4096, po/4096);
	printf("# of keys: %d\n", cur_page->num_keys);
	if (cur_page->is_leaf)
		*total_keys += cur_page->num_keys;
	printf("{");
	for (int i = 0; i < cur_page->num_keys; i++) {
		if (cur_page->is_leaf) {
			printf("[%"PRId64":%s] ", cur_page->records[i].key, cur_page->records[i].value);
		}
		else {
			printf("%"PRId64" ", cur_page->entries[i].key);
		}
	}
	printf("}\n");
}

void print_tree() {
	printf("\n\n\n");
	queue myQ;
	node *cur_page, *heightpage, *tmppage;
	off_t parent = 0, freepage, cur;
	int64_t total_keys = 0;
	int height = 0, num_internals = 0, num_fpage = 0;

	init_queue(&myQ);
	printf("fpo: %"PRId64"\nrpo: %"PRId64"\n", headerP->fpo/4096, headerP->rpo/4096);
	printf("# of pages: %"PRId64"\n\n", headerP->num_pages);

	freepage = headerP->fpo;
	while (freepage != 0) {
		num_fpage++;
		tmppage = open_page(freepage);
		freepage = tmppage->ppo;
	}

	if (headerP->rpo == 0) {
		printf("Empty Tree\n");
		printf("# of free page: %d\n", num_fpage);
		return;
	}


	heightpage = open_page(headerP->rpo);
	while(!heightpage->is_leaf) {
		height++;
		pread(db_fd, heightpage, PAGESIZE, heightpage->entries[0].npo);
	}
	free(heightpage);
	printf("tree height: %d\n", height);

	enqueue(&myQ, headerP->rpo);
	while(!is_empty(&myQ)) {
		cur = dequeue(&myQ);
		cur_page = open_page(cur);
		if(cur_page->ppo != parent) {
			printf("\n\nnext level\n");
			parent = cur_page->ppo;
		}
		print_page_info(cur_page, cur, &total_keys);
		if (!cur_page->is_leaf) {
			num_internals++;
			enqueue(&myQ, cur_page->expo);
			for (int i = 0; i < cur_page->num_keys; i++) {
				enqueue(&myQ, cur_page->entries[i].npo);
			}
		}
		free(cur_page);
	}

	printf("# of internal pages: %d, # of leaf pages: %"PRId64", # of free pages: %d\n", num_internals, headerP->num_pages - num_internals - num_fpage - 1, num_fpage);
	printf("total keys: %"PRId64"\n", total_keys);
}

void init_queue(queue *q)
{
	q->front = q->rear = NULL; //front와 rear를 NULL로 설정
	q->count = 0;//보관 개수를 0으로 설정
}
 
int is_empty(queue *q)
{
	return q->count == 0;
}
 
void enqueue(queue *q, off_t data)
{
	qnode *now = (qnode *)malloc(sizeof(qnode));
	now->po = data;
	now->next = NULL;
 
	if (is_empty(q))
	{
		q->front = now;
	}
	else
	{
		q->rear->next = now;
	}
	q->rear = now;
	q->count++;
}
 
off_t dequeue(queue *q)
{
	off_t cpo = 0;
	qnode *now;
	if (is_empty(q))
	{
		return cpo;
	}
	now = q->front;
	cpo = now->po;
	q->front = now->next;
	free(now);
	q->count--;
	return cpo;
}