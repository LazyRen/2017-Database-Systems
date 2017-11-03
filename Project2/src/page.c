#include "page.h"

int db_fd = -1;

int open_db(char *pathname) {
	int temp;
	off_t test;
	db_fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0777);
	headerP = (header_page*)calloc(1, PAGESIZE);

	if (db_fd > 0) {
		// printf("DB File successfully created\n");
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
		// printf("Existing DB File has been successfully opened\n");
		temp = pread(db_fd, headerP, PAGESIZE, SEEK_SET);
		if (temp < PAGESIZE) {
			printf("Failed to read header_page\n");
			exit(EXIT_FAILURE);
		}
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

//get_free_page is the only function that actually writes new page
//into the DB file. If there is free page available, it returns free page,
//if not, create new page and writes it and return the new page.
page* get_free_page(off_t ppo, off_t *page_loc, int is_leaf)
{
	int temp = 0;
	off_t fpo = headerP->fpo;
	page* new_page = (page*)calloc(1, PAGESIZE);
	if (fpo == 0) {//no free page avail. Make new page
		new_page->ppo = ppo; new_page->is_leaf = is_leaf;
		*page_loc = lseek(db_fd, 0, SEEK_END);
		temp = pwrite(db_fd, new_page, PAGESIZE, PAGESIZE * headerP->num_pages);
		if (temp < PAGESIZE) {
			printf("Failed to write from get_free_page()\n");
			exit(EXIT_FAILURE);
		}
		
		headerP->num_pages++;
		temp = pwrite(db_fd, headerP, PAGESIZE, SEEK_SET);
		if (temp < PAGESIZE) {
			printf("Failed to update headerP from get_free_page()\n");
			exit(EXIT_FAILURE);
		}
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


//if the page has 0 entries/records
//set all data as 0 and move it to the free page list
void add_free_page(off_t page_loc)
{
	page *new_free_page = calloc(1, PAGESIZE);
	new_free_page->ppo = headerP->fpo;
	pwrite(db_fd, new_free_page, PAGESIZE, page_loc);

	headerP->fpo = page_loc;
	pwrite(db_fd, headerP, PAGESIZE, 0);
	free(new_free_page);
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