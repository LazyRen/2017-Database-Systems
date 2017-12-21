#ifndef __LOG_RECOVERY_H__
#define __LOG_RECOVERY_H__

#include "structure.h"
#include "page.h"
#include "bpt.h"

#define LOGSIZE (296)

log_manager log_man;

//Functions for Transaction - Revory 
int begin_transaction();
int commit_transaction();
int abort_transaction();
void recover(log_structure *cur_log);
void undo_txn(off_t lsn);

//Functions for Log Files
log_structure* create_log(int type);
log_structure* open_log(off_t lsn);
void add_log(log_structure *new_log);
void write_log(off_t lsn);
void show_log_manager();

#endif