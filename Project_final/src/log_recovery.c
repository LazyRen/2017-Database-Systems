#include "log_recovery.h"

int begin_transaction()
{
	if (log_man.current_trx_id != -1) {
		fprintf(stderr, "transaction %d is already running\n", log_man.current_trx_id);
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
		fprintf(stderr, "No transaction to commit\n");
		return -1;
	}

	log_structure *tmp_structure = create_log(COMMIT);

	add_log(tmp_structure);
	write_log(tmp_structure->lsn);
	log_man.current_trx_id = -1;

	return 0;
}

int abort_transaction()
{
	if (log_man.current_trx_id == -1) {
		fprintf(stderr, "No transaction to abort\n");
		return -1;
	}

	log_structure *tmp_structure = create_log(ABORT);
	add_log(tmp_structure);

	undo_trx(tmp_structure->plsn);
	log_man.current_trx_id = -1;
	return 0;
}

void recover(log_structure *cur_log)
{
	off_t cpo = cur_log->cpn * PAGESIZE;
	int prev_trx_id = log_man.current_trx_id;
	buffer_structure *tmp = open_page(cur_log->table_id, cpo);
	if (tmp->lsn >= cur_log->lsn)
		return;

	log_man.current_trx_id = -1;
	update(cur_log->table_id, cur_log->old_key, cur_log->new_image);
	log_man.current_trx_id = prev_trx_id;

	return;
}

void undo_trx(off_t lsn)
{
	log_structure *to_undo = open_log(lsn);
	log_man.current_trx_id = to_undo->trx_id;
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

		free(to_undo);
		to_undo = open_log(to_undo->plsn);
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
	log_structure *tmp_structure  = log_man.log_spt;

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
	if (log_man.log_spt == NULL) {
		log_man.log_spt = new_log;
		log_man.log_ept = new_log;
		new_log->prev = NULL;
		new_log->next = NULL;
	}
	else {
		new_log->next = NULL;
		new_log->prev = log_man.log_ept;
		log_man.log_ept->next = new_log;
		log_man.log_ept = new_log;
	}

	return;
}

void write_log(off_t lsn)
{
	log_structure *tmp_structure;

	while(log_man.log_spt->lsn <= lsn && log_man.log_spt != NULL) {
		tmp_structure = log_man.log_spt;
		pwrite(log_man.fd, tmp_structure, LOGSIZE, tmp_structure->lsn - LOGSIZE);
		log_man.flushed_lsn = tmp_structure->lsn;
		log_man.log_spt = tmp_structure->next;
		log_man.log_spt->prev = NULL;
		free(tmp_structure);
	}

	if (log_man.log_spt == NULL)
		log_man.log_ept = NULL;

	return;
}