#include "page.h"

int init_db (int num_buf)
{
	buf_man.capacity = num_buf;
	buf_man.last_buf = 0;
	buf_man.buffer_pool = calloc(num_buf, sizeof(buffer_structure));
	if (buf_man.buffer_pool == NULL)
		return -1;
	for (int i = 0; i < num_buf; i++) {
		buf_man.buffer_pool[i].tid = -1;
		buf_man.buffer_pool[i].cpo = -1;
	}
	for (int i = 0; i < MAX_TABLE; i++) {
		table[i].fd = -1;
	}
	return 0;
}

int shutdown_db(void)
{
	for (int i = 0; i < buf_man.capacity; i++)
		if (buf_man.buffer_pool[i].is_dirty)
			write_buffer(&buf_man.buffer_pool[i]);
	free(buf_man.buffer_pool);

	return 0;
}

int open_table(char *pathname)
{
	int temp, tid;
	for (tid = 0; tid < MAX_TABLE; tid++) {
		if (table[tid].fd == -1)
			break;
	}
	if (tid == MAX_TABLE) {
		printf("Table is full\n");
		return -1;
	}
	table[tid].fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0777);
	header_page *headerP = (header_page*)calloc(1, PAGESIZE);

	if (table[tid].fd > 0) {
		headerP->num_pages = 1;
		temp = pwrite(table[tid].fd, headerP, PAGESIZE, SEEK_SET);
		if (temp < PAGESIZE) {
			printf("Failed to write header_page\n");
			table[tid].fd = -1;
			free(headerP);
			headerP = NULL;
			return -1;
		}

		free(headerP);
		return tid;
	}

	table[tid].fd = open(pathname, O_RDWR | O_SYNC);
	if (table[tid].fd > 0) {
		return tid;
	}

	printf("Failed to open %s\n", pathname);
	return -1;
}

int close_table(int table_id)
{
	if (table[table_id].fd == -1)
		return -1;
	for (int i = 0; i < buf_man.capacity; i++) {
		if (buf_man.buffer_pool[i].tid == table_id) {
			if (buf_man.buffer_pool[i].is_dirty)
				write_buffer(&buf_man.buffer_pool[i]);
			buf_man.buffer_pool[i].tid = -1;
			buf_man.buffer_pool[i].cpo = -1;
			buf_man.buffer_pool[i].is_dirty = false;
			buf_man.buffer_pool[i].refbit = false;
			buf_man.buffer_pool[i].pin_count = 0;
		}
	}
	table[table_id].fd = -1;
	return 0;
}

//read and open the page from the disk.
//must free page after use.
buffer_structure* open_page(int table_id, off_t po)
{
	buffer_structure *ret = NULL;
	int bid = buf_man.last_buf;
	for (int i = 0; i < buf_man.capacity; i++) {
		if (buf_man.buffer_pool[i].tid == table_id && buf_man.buffer_pool[i].cpo == po) {
			buf_man.buffer_pool[i].refbit = true;
			buf_man.buffer_pool[i].pin_count += 1;
			buf_man.last_buf = bid;
			// printf("tid: %d po: %"PRId64" found in buffer %d\n", table_id, po, i);
			return &buf_man.buffer_pool[i];
		}
	}

	while(ret == NULL) {
		bid = (bid + 1) % buf_man.capacity;
		if (buf_man.buffer_pool[bid].pin_count == 0 && buf_man.buffer_pool[bid].refbit == false) {
			if (buf_man.buffer_pool[bid].is_dirty)
				write_buffer(&buf_man.buffer_pool[bid]);
			pread(table[table_id].fd, &buf_man.buffer_pool[bid], PAGESIZE, po);
			buf_man.buffer_pool[bid].tid = table_id;
			buf_man.buffer_pool[bid].cpo = po;
			buf_man.buffer_pool[bid].is_dirty = false;
			buf_man.buffer_pool[bid].refbit = true;
			buf_man.buffer_pool[bid].pin_count = 1;
			buf_man.last_buf = bid;
			ret = &buf_man.buffer_pool[bid];
		}
		else if (buf_man.buffer_pool[bid].pin_count == 0 && buf_man.buffer_pool[bid].refbit == true)
			buf_man.buffer_pool[bid].refbit = false;
	}
	// printf("new buffer set at %d with tid: %d po: %"PRId64"\n", bid, table_id, po);
	return ret;
}

void write_buffer(buffer_structure *cur_buf)
{
	int temp;
	temp = pwrite(table[cur_buf->tid].fd, cur_buf, PAGESIZE, cur_buf->cpo);
	if (temp < PAGESIZE) {
		printf("Failed to write from buffer_structure()\n");
		exit(EXIT_FAILURE);
	}
	memset(cur_buf, 0, sizeof(buffer_structure));
	return;
}

void drop_pincount(buffer_structure *cur_buf, bool dirty)
{
	cur_buf->pin_count -= 1;
	if (!cur_buf->is_dirty)
		cur_buf->is_dirty = dirty;
}

void set_dirty(buffer_structure *cur_buf)
{
	cur_buf->is_dirty = true;
}

//get_free_page is the only function that actually writes new page
//into the DB file. If there is free page available, it returns free page,
//if not, create new page and writes it and return the new page.
buffer_structure* get_free_page(int table_id, off_t ppo, off_t *page_loc, int is_leaf)
{
	int bid = buf_man.last_buf;
	buffer_structure *headerP = open_page(table_id, SEEK_SET);
	off_t fpo = headerP->fpo;
	buffer_structure* new_page = NULL;
	page *temp_page;
	if (fpo == 0) {//no free page avail. Make new page
		*page_loc = PAGESIZE * headerP->num_pages;
		temp_page = calloc(1, PAGESIZE);
		pwrite(table[table_id].fd, temp_page, PAGESIZE, PAGESIZE * headerP->num_pages);
		free(temp_page);
		while(new_page == NULL) {
			bid = (bid + 1) % buf_man.capacity;
			if (buf_man.buffer_pool[bid].pin_count == 0 && buf_man.buffer_pool[bid].refbit == false) {
				if (buf_man.buffer_pool[bid].is_dirty)
					write_buffer(&buf_man.buffer_pool[bid]);
				pread(table[table_id].fd, &buf_man.buffer_pool[bid], PAGESIZE, *page_loc);
				buf_man.buffer_pool[bid].tid = table_id;
				buf_man.buffer_pool[bid].cpo = *page_loc;
				buf_man.buffer_pool[bid].is_dirty = true;
				buf_man.buffer_pool[bid].refbit = true;
				buf_man.buffer_pool[bid].pin_count = 1;
				buf_man.last_buf = bid;
				new_page = &buf_man.buffer_pool[bid];
			}
			else if (buf_man.buffer_pool[bid].pin_count == 0 && buf_man.buffer_pool[bid].refbit == true)
				buf_man.buffer_pool[bid].refbit = false;
		}
		new_page->ppo = ppo; new_page->is_leaf = is_leaf;
		
		headerP->num_pages++;
		drop_pincount(headerP, true);
	}
	else {//free page is avail.
		new_page = open_page(table_id, fpo);
		*page_loc = fpo; //disk loc. of read page
		fpo = new_page->ppo;
		new_page->ppo = ppo; //set ppo to calling page
		new_page->is_leaf = is_leaf;
		set_dirty(new_page);
		headerP->fpo = fpo;
		drop_pincount(headerP, true);
	}
	return new_page;
}


//if the page has 0 entries/records
//set all data as 0 and move it to the free page list
// void add_free_page(off_t page_loc)
// {
// 	page *new_free_page = calloc(1, PAGESIZE);
// 	new_free_page->ppo = headerP->fpo;
// 	pwrite(db_fd, new_free_page, PAGESIZE, page_loc);

// 	headerP->fpo = page_loc;
// 	pwrite(db_fd, headerP, PAGESIZE, 0);
// 	free(new_free_page);
// }

void show_buffer_manager(void)
{
	printf("Buffer Manager Info.\n");
	printf("capacity: %d\n", buf_man.capacity);
	for (int i = 0; i < buf_man.capacity; i++) {
		printf("\n");
		printf("%d buffer pool\n", i);
		printf("tid: %d cpo: %"PRId64"\n", buf_man.buffer_pool[i].tid,  buf_man.buffer_pool[i].cpo / PAGESIZE);
		printf("is_dirty: %d refbit: %d\n",  buf_man.buffer_pool[i].is_dirty,  buf_man.buffer_pool[i].refbit);
		printf("pin_count: %d\n",  buf_man.buffer_pool[i].pin_count);
		printf("\n");
	}
}
void print_page_info(buffer_structure *cur_page, off_t po, int64_t *total_keys)
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

void print_tree(int table_id) {
	printf("\n\n\n");
	queue myQ;
	buffer_structure *cur_page, *heightpage, *tmppage;
	buffer_structure *headerP = open_page(table_id, SEEK_SET);
	
	off_t parent = 0, freepage, cur;
	int64_t total_keys = 0;
	int height = 0, num_internals = 0, num_fpage = 0;

	init_queue(&myQ);
	printf("fpo: %"PRId64"\nrpo: %"PRId64"\n", headerP->fpo/4096, headerP->rpo/4096);
	printf("# of pages: %"PRId64"\n\n", headerP->num_pages);

	freepage = headerP->fpo;
	while (freepage != 0) {
		num_fpage++;
		tmppage = open_page(table_id, freepage);
		freepage = tmppage->ppo;
	}

	if (headerP->rpo == 0) {
		printf("Empty Tree\n");
		printf("# of free page: %d\n", num_fpage);
		return;
	}


	heightpage = open_page(table_id, headerP->rpo);
	while(!heightpage->is_leaf) {
		height++;
		drop_pincount(heightpage, false);
		heightpage = open_page(table_id, heightpage->entries[0].npo);
	}
	drop_pincount(heightpage, false);
	printf("tree height: %d\n", height);

	enqueue(&myQ, headerP->rpo);
	while(!is_empty(&myQ)) {
		cur = dequeue(&myQ);
		cur_page = open_page(table_id, cur);
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
		drop_pincount(cur_page, false);
	}

	printf("# of internal pages: %d, # of leaf pages: %"PRId64", # of free pages: %d\n", num_internals, headerP->num_pages - num_internals - num_fpage - 1, num_fpage);
	printf("total keys: %"PRId64"\n", total_keys);
	drop_pincount(headerP, false);
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