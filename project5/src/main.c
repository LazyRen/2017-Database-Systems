#include "page.h"
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#define bufnum 16
FILE *fp;

int main(int argc, char ** argv) {
	// int64_t key, t;
	// int tid[2], tnum;
	// char instruction;
	// char value[300];
	// int automate = 1;
	// init_db(bufnum);
	// srand(time(NULL));
	// tid[0] = open_table("DATA1");
	// // printf("automate db?(Y:1, N:0): ");
	// // scanf("%d", &automate);
	// //2^20 == 1048576 == (sizeof_db = 4GB)
	// // if (automate) {
	// // 	bool *check;
	// // 	int *dupli;
	// // 	bool nodup;
	// // 	int64_t inputnum, deletenum;

	// // 	printf("insert num: ");
	// // 	scanf("%"PRId64, &inputnum);
	// // 	check = malloc(sizeof(bool) * inputnum);
	// // 	dupli = malloc(sizeof(int) * inputnum);
	// // 	memset(check, false, sizeof(bool) * inputnum);
	// // 	memset(dupli, 0, sizeof(int) * inputnum);
	// // 	for (int i = inputnum / 2 + 1; i >= 1; i--) {
	// // 		do {
	// // 			nodup = true;
	// // 			t = rand() % inputnum;
	// // 			if (check[t])
	// // 				nodup = false;
	// // 		} while(!nodup);
	// // 		check[t] = true;
	// // 		dupli[t] += 1;
	// // 		sprintf(value, "%"PRId64, t);
	// // 		// printf("%lld %s\n", t, value);
	// // 		insert(tid[0], t, value);
	// // 	}
	// // 	memset(check, false, sizeof(bool) * inputnum);
	// // 	for (int i = inputnum / 2 + 1; i >= 1; i--) {
	// // 		do {
	// // 			nodup = true;
	// // 			t = rand() % inputnum;
	// // 			if (check[t])
	// // 				nodup = false;
	// // 		} while(!nodup);
	// // 		check[t] = true;
	// // 		dupli[t] += 1;
	// // 		sprintf(value, "%"PRId64, t);
	// // 		// printf("%lld %s\n", t, value);
	// // 		insert(tid[1], t, value);
	// // 	}
	// // }

	// 	// printf("delete num: ");
	// 	// scanf("%"PRId64, &deletenum);
	// 	// memset(check, false, sizeof(check));
	// 	// for (int i = deletenum; i >= 1; i--) {
	// 	// 	do {
	// 	// 		nodup = true;
	// 	// 		t = rand() % deletenum;
	// 	// 		if (check[t])
	// 	// 			nodup = false;
	// 	// 	}	while(!nodup);
	// 	// 	check[t] = true;
	// 	// 	t = i;
	// 	// 	// printf("delecting %"PRId64"\n", t);
	// 	// 	if (delete(tid, t) == -1) {
	// 	// 		print_tree(tid);
	// 	// 		exit(EXIT_FAILURE);
	// 	// 	}
	// 	// }
	// while(1) {
	// 	printf("s / c / a / i / u / f / t / b / l / d / j / q\n");
	// 	do 
	// 		instruction = getchar();
	// 	while(isspace(instruction));
	// 	printf("instruction: %c\n", instruction);
	// 	switch(instruction) {
	// 		case 's':
	// 			begin_transaction();
	// 			break;
	// 		case 'c':
	// 			commit_transaction();
	// 			break;
	// 		case 'a':
	// 			abort_transaction();
	// 			break;
	// 		case 'i':
	// 			scanf("%d %"PRId64" %s", &tnum, &key, value);
	// 			while(getchar() != '\n');
	// 			insert(tnum, key, value);
	// 			// printf("key: %lld\n\n", key);
	// 			break;
	// 		case 'u':
	// 			scanf("%d %"PRId64" %s", &tnum, &key, value);
	// 			while(getchar() != '\n');
	// 			update(tnum, key, value);
	// 			break;
	// 		case 'f':
	// 			scanf("%d %"PRId64, &tnum, &key);
	// 			while(getchar() != '\n');
	// 			find_and_print(tnum, key);
	// 			break;
	// 		case 't':
	// 			for (int i = 0; i <= 10; i++) {
	// 				if (table[i].fd != -1) {
	// 					printf("\n\nTable %d\n", i);
	// 					print_tree(i);
	// 				}
	// 			}
	// 			break;
	// 		case 'b':
	// 			show_buffer_manager();
	// 			break;
	// 		case 'l':
	// 			show_log_manager();
	// 			break;
	// 		case 'd':
	// 			scanf("%"PRId64, &key);
	// 			if (delete(tid[0], key) != -1)
	// 				printf("key: %"PRId64" deleted\n\n", key);
	// 			break;
	// 		case 'j':
	// 			break;
	// 		case 'q':
	// 			shutdown_db();
	// 			return EXIT_SUCCESS;
	// 		default:
	// 			while(getchar() != '\n');
	// 			printf("No such Instruction\n");
	// 	}
	// }
	// return EXIT_SUCCESS;

	int table_id;
	int size;
	int64_t input;
	char instruction;
	char buf[120];
	char path[120];
	init_db(16);

	while(scanf("%c", &instruction) != EOF){
		switch(instruction){
			case 'b':
				begin_transaction();
				break;
			case 'a' :
				abort_transaction();
				// print_tree(1);
				printf("abort done\n");
				fflush(stdout);
				break;
			case 'c':
				commit_transaction();
				printf("commit done\n");
				fflush(stdout);
				break;
			case 'd':
				scanf("%d %"PRId64"", &table_id, &input);
				delete(table_id, input);
				break;
			case 'i':
				scanf("%d %"PRId64" %s", &table_id, &input, buf);
				insert(table_id, input, buf);
				break;
			case 'u':
				scanf("%d %"PRId64" %s", &table_id, &input, buf);
				update(table_id, input, buf);
				break;
			case 'f':
				scanf("%d %"PRId64"", &table_id, &input);
				char * ftest;
				if((ftest = find(table_id, input)) != NULL) {
					printf("Key: %"PRId64", Value: %s\n", input, ftest);
					fflush(stdout);
					free(ftest);
				}
				else {
					printf("Not Exists\n");
					fflush(stdout);
				}
				break;
			case 'n':
				scanf("%d", &size);
				init_db(size);
			break;
			case 'o':
				scanf("%s", path);
				table_id = open_table(path);
				printf("%s opened\n", path);
				fflush(stdout);
				break;
			case 't':
				print_tree(1);
				printf("done\n");
				fflush(stdout);
				break;
			case 'q':
				shutdown_db();
				printf("exiting\n");
				fflush(stdout);
				return 0;
				while(getchar() != (int64_t)'\n');
				break;
		}
		while(getchar() != (int)'\n');
	}
	printf("\n");
	fflush(stdout);
	return 0;

}