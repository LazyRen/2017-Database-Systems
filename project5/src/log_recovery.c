#include "log_recovery.h"

int begin_transaction()
{
	if (log_man.current_trx_id != -1) {
		// fprintf(stderr, "transaction %d is already running\n", log_man.current_trx_id);
		return -1;
	}

	log_man.current_trx_id = ++log_man.last_trx_id;
	log_structure *tmp_structure = create_log(BEGIN);

	add_log(tmp_structure);

	return 0;
}

int commit_transaction()
{
	if (log_man.current_trx_id == -1) {
		// fprintf(stderr, "No transaction to commit\n");
		return -1;
	}

	log_structure *tmp_structure = create_log(COMMIT);

	add_log(tmp_structure);
	write_log(log_man.last_lsn);
	log_man.current_trx_id = -1;
	return 0;
}

int abort_transaction()
{
	if (log_man.current_trx_id == -1) {
		// fprintf(stderr, "No transaction to abort\n");
		return -1;
	}

	log_structure *tmp_structure = create_log(ABORT);
	add_log(tmp_structure);

	undo_txn(tmp_structure->plsn);
	log_man.current_trx_id = -1;
	write_log(log_man.last_lsn);
	
	return 0;
}

void recover(log_structure *cur_log)
{
	// fprintf(stderr, "\n");
	off_t cpo = cur_log->cpn * PAGESIZE;
	int prev_trx_id = log_man.current_trx_id;
	buffer_structure *tmp = open_page(cur_log->table_id, cpo);
	if (tmp->lsn >= cur_log->lsn) {
		// fprintf(stderr, "consider recover %"PRId64"\n", cur_log->lsn);
		drop_pincount(tmp, false);
		return;
	}

	log_man.current_trx_id = -1;
	// fprintf(stderr, "key: %"PRId64"\n %s to %s\n", cur_log->old_key, cur_log->old_image, cur_log->new_image);
	update(cur_log->table_id, cur_log->old_key, cur_log->new_image);
	tmp->lsn = cur_log->lsn;
	drop_pincount(tmp, true);
	log_man.current_trx_id = prev_trx_id;

	return;
}

void undo_txn(off_t lsn)
{
	log_structure *to_undo = open_log(lsn);
	log_man.current_trx_id = to_undo->trx_id;
	off_t tmp_lsn;
	while(to_undo != NULL) {
		if (to_undo->type == BEGIN || to_undo->type == ABORT) {
			free(to_undo);
			break;
		}
		log_structure *undo_log = create_log(UPDATE);
		undo_log->table_id = to_undo->table_id;
		undo_log->cpn = to_undo->cpn;
		undo_log->so = to_undo->so;
		undo_log->data_len = to_undo->data_len;
		undo_log->old_key = to_undo->new_key;
		strcpy(undo_log->old_image, to_undo->new_image);
		undo_log->new_key = to_undo->old_key;
		strcpy(undo_log->new_image, to_undo->old_image);
		add_log(undo_log);

		//actual undo
		recover(undo_log);

		tmp_lsn = to_undo->plsn;
		free(to_undo);
		to_undo = open_log(tmp_lsn);
	}

	log_structure *tmp_structure = create_log(END);
	add_log(tmp_structure);

	log_man.current_trx_id = -1;
}

log_structure* create_log(int type)
{
	log_structure *new_log = calloc(1, sizeof(log_structure));
	new_log->lsn = log_man.last_lsn + LOGSIZE;
	if (type == BEGIN)
		new_log->plsn = 0;
	else
		new_log->plsn = log_man.last_lsn;
	new_log->trx_id = log_man.current_trx_id;
	new_log->type = type;
	new_log->data_len = LOGSIZE;
	log_man.last_lsn += LOGSIZE;

	return new_log;
}

//open the log from the log file or from in-memory log
//must free log_record structure after use
log_structure* open_log(off_t lsn)
{
	bool in_memory = false;
	log_structure *temp_log = (log_structure*)calloc(1, sizeof(log_structure));
	log_structure *tmp_structure  = log_man.head;

	if (lsn <= 0) {
		free(temp_log);
		return NULL;
	}

	while(tmp_structure != NULL) {
		if (tmp_structure->lsn == lsn) {
			memcpy(temp_log, tmp_structure, sizeof(log_structure));
			in_memory = true;
			break;
		}
		tmp_structure = tmp_structure->next;
	}
	if (!in_memory)
		pread(log_man.fd, temp_log, LOGSIZE, lsn - LOGSIZE);

	return temp_log;
}

void add_log(log_structure *new_log)
{
	if (log_man.head == NULL) {
		log_man.head = new_log;
		log_man.tail = new_log;
		new_log->prev = NULL;
		new_log->next = NULL;
	}
	else {
		new_log->next = NULL;
		new_log->prev = log_man.tail;
		log_man.tail->next = new_log;
		log_man.tail = new_log;
	}

	return;
}

void write_log(off_t lsn)
{
	log_structure *tmp_structure = log_man.head;
	if (tmp_structure == NULL)
		return;
	while(tmp_structure->lsn <= lsn) {
		pwrite(log_man.fd, tmp_structure, LOGSIZE, tmp_structure->lsn - LOGSIZE);
		log_man.flushed_lsn = tmp_structure->lsn;
		if (tmp_structure->next != NULL) {
			log_man.head = tmp_structure->next;
			log_man.head->prev = NULL;
			free(tmp_structure);
			tmp_structure = log_man.head;
		}
		else {
			log_man.head = log_man.tail = NULL;
			free(tmp_structure);
			break;
		}
	}
	// fprintf(stderr, "end write_log\n");
	return;
}

void show_log_manager()
{
	printf("flushed_lsn : %"PRId64"\n", log_man.flushed_lsn);
	printf("last_lsn : %"PRId64"\n", log_man.last_lsn);
	printf("current trx id: %d\n", log_man.current_trx_id);
	printf("last trx id: %d\n\n", log_man.last_trx_id);

	log_structure *tmp_log = log_man.head;
	while(tmp_log != NULL) {
		printf("lsn: %"PRId64"\n", tmp_log->lsn);
		printf("plsn: %"PRId64"\n", tmp_log->plsn);
		printf("trx id: %d\n", tmp_log->trx_id);
		printf("type: %d\n", tmp_log->type);
		printf("table id: %d\n", tmp_log->table_id);
		printf("cpn: %d\nso: %d\n", tmp_log->cpn, tmp_log->so);

		tmp_log = tmp_log->next;
		printf("\n");
	}
}