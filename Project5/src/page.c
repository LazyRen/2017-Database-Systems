#include "page.h"

int init_db (int num_buf)
{
	buf_man.capacity = num_buf;
	buf_man.last_buf = -1;
	if (num_buf >= 100 && toggle_bs)
		binary_search = true;
	else
		binary_search = false;
	if (binary_search)
		for (int i = 0; i < MAX_TABLE; i++)
			buf_man.buffer_lookup[i] = calloc(num_buf, sizeof(buf_lookup));

	buf_man.buffer_pool = calloc(num_buf, sizeof(buffer_structure));
	buf_man.hash_table = calloc(num_buf, sizeof(buffer_hashframe));
	if (buf_man.buffer_pool == NULL)
		return -1;
	for (int i = 0; i < num_buf; i++) {
		buf_man.buffer_pool[i].tid = -1;
		buf_man.buffer_pool[i].cpo = -1;
		buf_man.hash_table[i].tid = -1;
		buf_man.hash_table[i].cpo = -1;
		buf_man.hash_table[i].prev = NULL;
		buf_man.hash_table[i].next = NULL;
	}
	for (int i = 0; i < MAX_TABLE; i++) {
		buf_man.table_size[i] = 0;
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
	free(buf_man.hash_table);
	if (binary_search)
		for (int i = 0; i < MAX_TABLE; i++) {
			free(buf_man.buffer_lookup[i]);
			if (table[i].fd != -1)
				close(table[i].fd);
		}

	return 0;
}

int open_table(char *pathname)
{
	int temp, tid;
	tid = pathname[strlen(pathname) - 2] - '0';
	if (tid > 10 || tid < 0) {
		printf("failed to retrieve tid\n");
		return -1;
	}
	if (table[tid].fd != -1) {
		printf("Table is already occupied\n");
		printf("Make sure to call close_table() for data[%d]\n", tid);
		return -1;
	}
	table[tid].fd = open(pathname, O_RDWR | O_CREAT | O_EXCL, 0777);
	buf_man.table_size[tid] = 0;
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

	table[tid].fd = open(pathname, O_RDWR);
	if (table[tid].fd > 0)
		return tid;

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
			buf_man.table_size[i] = 0;
		}
	}
	close(table[table_id].fd);
	table[table_id].fd = -1;
	return 0;
}

int join_table(int table_id_1, int table_id_2, char *pathname)
{
	FILE *result_fp = fopen(pathname, "w");
	if (result_fp == NULL) {
		printf("failed to open join result file\n");
		return -1;
	}
	int64_t count_result = 0;
	buffer_structure *headerP_1 = open_page(table_id_1, SEEK_SET);
	buffer_structure *headerP_2 = open_page(table_id_2, SEEK_SET);

	if (headerP_1->rpo == 0 || headerP_2->rpo == 0) {
		printf("One table is empty\n");
		return -1;
	}
	buffer_structure *table_1 = open_page(table_id_1, headerP_1->rpo);
	buffer_structure *table_2 = open_page(table_id_2, headerP_2->rpo);
	drop_pincount(headerP_1, false);
	drop_pincount(headerP_2, false);
	
	//get left most leaf page from both tables
	while (!table_1->is_leaf) {
		off_t npo = table_1->expo;
		drop_pincount(table_1, false);
		table_1 = open_page(table_id_1, npo);
	}
	while (!table_2->is_leaf) {
		off_t npo = table_2->expo;
		drop_pincount(table_2, false);
		table_2 = open_page(table_id_2, npo);
	}

	buffer_structure *result_cache = get_avail_buffer();
	bool done = false;
	int r = 0, s = 0, num_keys = 0;
	off_t npo;
	while (true) {
		while (table_1->records[r].key < table_2->records[s].key) {
			r++;
			if (table_1->num_keys <= r) {
				if (table_1->expo == 0) {
					done = true;
					break;
				}
				else {
					npo = table_1->expo;
					drop_pincount(table_1, false);
					table_1 = open_page(table_id_1, npo);
					r = 0;
				}
			}
		}
		if (done) break;

		while (table_1->records[r].key > table_2->records[s].key) {
			s++;
			if (table_2->num_keys <= s) {
				if (table_2->expo == 0) {
					done = true;
					break;
				}
				else {
					npo = table_2->expo;
					drop_pincount(table_2, false);
					table_2 = open_page(table_id_2, npo);
					s = 0;
				}
			}
		}
		if (done) break;

		if (table_1->records[r].key == table_2->records[s].key) {
			count_result += 1;
			result_cache->join_a[num_keys].key = table_1->records[r].key;
			strcpy(result_cache->join_a[num_keys].value, table_1->records[r].value);
			result_cache->join_b[num_keys].key = table_2->records[s].key;
			strcpy(result_cache->join_b[num_keys].value, table_2->records[s].value);
			num_keys++;
			if (num_keys == 16) {
				flush_result(result_fp, result_cache, num_keys);
				result_cache = get_avail_buffer();
				num_keys = 0;
			}
			r++;
		}
	}
	flush_result(result_fp, result_cache, num_keys);
	fclose(result_fp);
	// printf("%"PRId64" joined\n", count_result);
	return 0;
}

buffer_structure* get_avail_buffer()
{
	buffer_structure *ret = NULL;
	int bid = buf_man.last_buf;

	while(ret == NULL) {
		bid = (bid + 1) % buf_man.capacity;
		if (buf_man.buffer_pool[bid].pin_count == 0 && buf_man.buffer_pool[bid].refbit == false) {
			if (binary_search)
				delete_buffer(buf_man.buffer_pool[bid].tid, buf_man.buffer_pool[bid].cpo);
			delete_hash(buf_man.buffer_pool[bid].tid, buf_man.buffer_pool[bid].cpo);
			if (buf_man.buffer_pool[bid].is_dirty)
				write_buffer(&buf_man.buffer_pool[bid]);
			memset(&buf_man.buffer_pool[bid], 0, PAGESIZE);

			buf_man.buffer_pool[bid].tid = -1;
			buf_man.buffer_pool[bid].cpo = -1;
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

void flush_result(FILE *result_fp, buffer_structure *result_cache, int num_keys)
{

	for (int i = 0; i < num_keys; i++) {
		fprintf(result_fp, "%"PRId64",%s,", result_cache->join_a[i].key, result_cache->join_a[i].value);
		fprintf(result_fp, "%"PRId64",%s\n", result_cache->join_b[i].key, result_cache->join_b[i].value);
	}

	memset(result_cache, 0, PAGESIZE);
	result_cache->refbit = false;
	result_cache->pin_count = 0;
	return;
}

//read and open the page from the disk.
//must free page after use.
//If page is alread in buffer pool, simpl returns it
//If not, locate page from the disk and read it.
//If buffer pool is full, find idle buffer page based on CLOCK policy and change it.
buffer_structure* open_page(int table_id, off_t po)
{
	buffer_structure *ret = NULL;
	int bid = buf_man.last_buf;
	int hashing = (po/PAGESIZE) % buf_man.capacity;
	buffer_hashframe *hf;

	if (binary_search) {
		int loc = bs_buffer(table_id, po);
		if (loc != -1) {
			buf_man.buffer_pool[loc].refbit = true;
			buf_man.buffer_pool[loc].pin_count += 1;
			return &buf_man.buffer_pool[loc];
		}
	}
	// else {
	// 	for (int i = 0; i < buf_man.capacity; i++) {
	// 		if (buf_man.buffer_pool[i].tid == table_id && buf_man.buffer_pool[i].cpo == po) {
	// 			buf_man.buffer_pool[i].refbit = true;
	// 			buf_man.buffer_pool[i].pin_count += 1;
	// 			return &buf_man.buffer_pool[i];
	// 		}
	// 	}
	// }
	else {
		hf = &buf_man.hash_table[hashing];
		int loc = -1;
		while(hf != NULL) {
			if (hf->tid == table_id && hf->cpo == po) {
				loc = hf->buf_loc;
				break;
			}
			hf = hf->next;
		}
		if (loc != -1) {
			buf_man.buffer_pool[loc].refbit = true;
			buf_man.buffer_pool[loc].pin_count += 1;
			return &buf_man.buffer_pool[loc];
		}
	}


	while(ret == NULL) {
		bid = (bid + 1) % buf_man.capacity;
		if (buf_man.buffer_pool[bid].pin_count == 0 && buf_man.buffer_pool[bid].refbit == false) {
			if (binary_search) {
				if (buf_man.buffer_pool[bid].tid == table_id)
					modify_buffer(table_id, buf_man.buffer_pool[bid].cpo, po, bid);
				else {
					delete_buffer(buf_man.buffer_pool[bid].tid, buf_man.buffer_pool[bid].cpo);
					insert_buffer(table_id, po, bid);
				}
			}
			delete_hash(buf_man.buffer_pool[bid].tid, buf_man.buffer_pool[bid].cpo);
			insert_hash(table_id, po, bid);
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

//On the event of exchanging buffer pool, if buffer page is dirty, write it back to disk.
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
	buffer_structure *headerP = open_page(table_id, SEEK_SET);
	off_t fpo = headerP->fpo;
	buffer_structure* new_page = NULL;
	page *temp_page;
	if (fpo == 0) {//no free page avail. Make new page
		*page_loc = PAGESIZE * headerP->num_pages;
		temp_page = calloc(1, PAGESIZE);
		pwrite(table[table_id].fd, temp_page, PAGESIZE, PAGESIZE * headerP->num_pages);
		free(temp_page);
		new_page = open_page(table_id, *page_loc);
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
void add_free_page(int table_id, off_t page_loc)
{
	buffer_structure *headerP = open_page(table_id, SEEK_SET);
	buffer_structure *new_free_page = open_page(table_id, page_loc);
	memset(new_free_page, 0, PAGESIZE);
	new_free_page->ppo = headerP->fpo;
	

	headerP->fpo = page_loc;
	drop_pincount(headerP, true);
	drop_pincount(new_free_page, true);
}

void insert_hash(int tid, int64_t cpo, int loc)
{
	int hashing = (cpo/PAGESIZE) % buf_man.capacity;
	buffer_hashframe *new_hf = malloc(sizeof(buffer_hashframe));
	buffer_hashframe *temp_hf = buf_man.hash_table[hashing].next;

	new_hf->tid = tid; new_hf->cpo = cpo; new_hf->buf_loc = loc;
	if (temp_hf != NULL)
		temp_hf->prev = new_hf;
	new_hf->next = buf_man.hash_table[hashing].next;
	new_hf->prev = &buf_man.hash_table[hashing];
	buf_man.hash_table[hashing].next = new_hf;
}

void delete_hash(int tid, int64_t cpo)
{
	// printf("delete_buffer(%d %"PRId64")\n", tid, cpo); 
	if (tid == -1 || cpo == -1)
		return;
	int hashing = (cpo/PAGESIZE) % buf_man.capacity;
	buffer_hashframe *hf = buf_man.hash_table[hashing].next;
	while (hf != NULL) {
		if (hf->tid == tid && hf->cpo == cpo)
			break;
		hf = hf->next;
	}
	buffer_hashframe *prevhf = hf->prev, *nexthf = hf->next;
	prevhf->next = hf->next;
	if (nexthf != NULL)
		nexthf->prev = hf->prev;

	free(hf);
}

//binary search on buffer to locate page if exist
int bs_buffer(int tid, int64_t cpo)
{
	int l = 0, m , h = buf_man.table_size[tid] - 1;
	int ret = -1;
	while (l <= h && ret == -1) {
		m = (h + l) / 2;
		if (buf_man.buffer_lookup[tid][m].cpo == cpo)
			ret = buf_man.buffer_lookup[tid][m].buf_loc;
		else if (buf_man.buffer_lookup[tid][m].cpo < cpo)
			l = m + 1;
		else
			h = m - 1;
	}
	// printf("bs_buffer ret: %d\n", ret);
	return ret;
}

//sort function for the binary search
void insert_buffer(int tid, int64_t cpo, int loc)
{
	int i = buf_man.table_size[tid] - 1;
	if (buf_man.table_size[tid] == 0) {
		buf_man.buffer_lookup[tid][0].cpo = cpo;
		buf_man.buffer_lookup[tid][0].buf_loc = loc;
	}
	else {
		while (0 <= i) {
			if (buf_man.buffer_lookup[tid][i].cpo > cpo) {
				buf_man.buffer_lookup[tid][i + 1].cpo = buf_man.buffer_lookup[tid][i].cpo;
				buf_man.buffer_lookup[tid][i + 1].buf_loc = buf_man.buffer_lookup[tid][i].buf_loc;
			}
			else {
				break;
			}
			i--;
		}
		buf_man.buffer_lookup[tid][i + 1].cpo = cpo;
		buf_man.buffer_lookup[tid][i + 1].buf_loc = loc;
	}
	// printf("insert_buffer(%d %"PRId64" %d) at %d\n", tid, cpo, loc, i + 1);
	buf_man.table_size[tid] += 1;
}

//sort function for the binary search
void delete_buffer(int tid, int64_t cpo)
{
	// printf("delete_buffer(%d %"PRId64")\n", tid, cpo);
	if (tid == -1 || cpo == -1)
		return;
	int loc = -1;
	int l = 0, m, h = buf_man.table_size[tid] - 1;

	while (l <= h && loc == -1) {
		m = (h + l) / 2;
		if (buf_man.buffer_lookup[tid][m].cpo == cpo)
			loc = m;
		else if (buf_man.buffer_lookup[tid][m].cpo < cpo)
			l = m + 1;
		else
			h = m - 1;
	}

	if (loc == -1) {
		printf("delete_buffer error\n");
		exit(EXIT_FAILURE);
	}

	while (loc < buf_man.table_size[tid] - 1) {
		buf_man.buffer_lookup[tid][loc].cpo = buf_man.buffer_lookup[tid][loc + 1].cpo;
		buf_man.buffer_lookup[tid][loc].buf_loc = buf_man.buffer_lookup[tid][loc + 1].buf_loc;
		loc++;
	}

	buf_man.table_size[tid] -= 1;
}

//sort function for the binary search
void modify_buffer(int tid, int64_t old_cpo, int64_t new_cpo, int bid)
{
	int loc = -1;
	int l = 0, m, h = buf_man.table_size[tid] - 1;

	while (l <= h && loc == -1) {
		m = (h + l) / 2;
		if (buf_man.buffer_lookup[tid][m].cpo == old_cpo)
			loc = m;
		else if (buf_man.buffer_lookup[tid][m].cpo < old_cpo)
			l = m + 1;
		else
			h = m - 1;
	}

	if (loc == -1) {
		printf("delete_buffer error\n");
		exit(EXIT_FAILURE);
	}


	while (loc < buf_man.table_size[tid] - 1 && buf_man.buffer_lookup[tid][loc + 1].cpo < new_cpo) {
		buf_man.buffer_lookup[tid][loc].cpo = buf_man.buffer_lookup[tid][loc + 1].cpo;
		buf_man.buffer_lookup[tid][loc].buf_loc = buf_man.buffer_lookup[tid][loc + 1].buf_loc;
		loc++;
	}
	while (loc > 0 && buf_man.buffer_lookup[tid][loc - 1].cpo > new_cpo) {
		buf_man.buffer_lookup[tid][loc].cpo = buf_man.buffer_lookup[tid][loc - 1].cpo;
		buf_man.buffer_lookup[tid][loc].buf_loc = buf_man.buffer_lookup[tid][loc - 1].buf_loc;
		loc--;
	}
	buf_man.buffer_lookup[tid][loc].cpo = new_cpo;
	buf_man.buffer_lookup[tid][loc].buf_loc = bid;
}

void show_buffer_manager(void)
{
	printf("Buffer Manager Info.\n");
	printf("capacity: %d\n", buf_man.capacity);
	for (int i = 0; i < buf_man.capacity; i++) {
		printf("\n");
		printf("%d buffer pool\n", i);
		printf("tid: %d cpo: %"PRId64"\n", buf_man.buffer_pool[i].tid,  buf_man.buffer_pool[i].cpo);
		printf("is_dirty: %d refbit: %d\n",  buf_man.buffer_pool[i].is_dirty,  buf_man.buffer_pool[i].refbit);
		printf("pin_count: %d\n",  buf_man.buffer_pool[i].pin_count);
		printf("\n");
	}
}

void print_page_info(buffer_structure *cur_page, int64_t *total_keys)
{
	printf("%s page at %"PRId64" - %"PRId64"\n", cur_page->is_leaf ? "leaf" : "internal", cur_page->ppo/4096, cur_page->cpo/4096);
	printf("# of keys: %d\n", cur_page->num_keys);
	if (cur_page->is_leaf && total_keys != NULL)
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
		drop_pincount(tmppage, false);
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
		print_page_info(cur_page, &total_keys);
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