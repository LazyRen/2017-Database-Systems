#include "page.h"
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#define bufnum 16
FILE *fp;

int main(int argc, char ** argv) {
	// int64_t key, t, count_result = 0;
	// int tid[2];
	// char instruction;
	// char value[300];
	// int automate = 1;
	// init_db(bufnum);
	// srand(time(NULL));
	// tid[0] = open_table("./mydb1.db");
	// tid[1] = open_table("./mydb2.db");
	// printf("automate db?(Y:1, N:0): ");
	// scanf("%d", &automate);
	// //2^20 == 1048576 == (sizeof_db = 4GB)
	// if (automate) {
	// 	bool check[32768];
	// 	int dupli[32768];
	// 	bool nodup;
	// 	int64_t inputnum, deletenum;

	// 	printf("insert num: ");
	// 	scanf("%"PRId64, &inputnum);
	// 	memset(check, false, sizeof(check));
	// 	memset(dupli, 0, sizeof(dupli));
	// 	for (int i = inputnum / 2 + 1; i >= 1; i--) {
	// 		do {
	// 			nodup = true;
	// 			t = rand() % inputnum;
	// 			if (check[t])
	// 				nodup = false;
	// 		}	while(!nodup);
	// 		check[t] = true;
	// 		dupli[t] += 1;
	// 		// t = i;
	// 		sprintf(value, "%"PRId64, t);
	// 		// printf("%lld %s\n", t, value);
	// 		insert(tid[0], t, value);
	// 	}
	// 	// printf("a\n");
	// 	// for (int i = 0; i < inputnum; i++) {
	// 	// 	if (check[i])
	// 	// 		printf("%d ", i);
	// 	// }
	// 	// printf("\n");
	// 	memset(check, false, sizeof(check));
	// 	for (int i = inputnum / 2 + 1; i >= 1; i--) {
	// 		do {
	// 			nodup = true;
	// 			t = rand() % inputnum;
	// 			if (check[t])
	// 				nodup = false;
	// 		}	while(!nodup);
	// 		check[t] = true;
	// 		dupli[t] += 1;
	// 		// t = i;
	// 		sprintf(value, "%"PRId64, t);
	// 		// printf("%lld %s\n", t, value);
	// 		insert(tid[1], t, value);
	// 	}
	// 	// printf("b\n");
	// 	// for (int i = 0; i < inputnum; i++) {
	// 	// 	if (check[i])
	// 	// 		printf("%d ", i);
	// 	// }
	// 	// printf("\n");
	// 	join_table(tid[0], tid[1], "./result.txt");
	// 	for (int i = 0; i < inputnum; i++) {
	// 		if (dupli[i] == 2) {
	// 			count_result += 1;
	// 			printf("%d ", i);
	// 		}
	// 	}
	// 	printf("\n%"PRId64" dupicated\n", count_result);
	// 	shutdown_db();

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
	// }
	// while(1) {
	// 	printf("i / f / t / b / d / j / q\n");
	// 	do 
	// 		instruction = getchar();
	// 	while(isspace(instruction));
	// 	printf("instruction: %c\n", instruction);
	// 	switch(instruction) {
	// 		case 'i':
	// 			scanf("%"PRId64" %s", &key, value);
	// 			while(getchar() != '\n');
	// 			insert(tid, key, value);
	// 			// printf("key: %lld\n\n", key);
	// 			break;
	// 		case 'f':
	// 			scanf("%"PRId64, &key);
	// 			while(getchar() != '\n');
	// 			find_and_print(tid, key);
	// 			break;
	// 		case 't':
	// 			print_tree(tid);
	// 			break;
	// 		case 'b':
	// 			show_buffer_manager();
	// 			break;
	// 		case 'd':
	// 			scanf("%"PRId64, &key);
	// 			if (delete(tid, key) != -1)
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

	int table = 0;
	int64_t key;
	char buf[255];
	char cmd = 0;
	char *tok;
	char * ret;
 
	init_db(4000);
   
	// no use buffer stdin, stdout
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
 
 
	while(1) {
		memset(buf, 0, 255);
		cmd = getchar();
		getchar();
		switch(cmd) {
			case 'o':
				if (fgets(buf, 255, stdin) != NULL) {
					buf[strlen(buf)-1] = '\0';
					printf("%d\n", open_table(buf));
				}
				break;
			case 'f': // find
				if (fgets(buf, 255, stdin) != NULL) {
					tok = strtok(buf, " \n");
					table = atoi(buf);
					tok = strtok(NULL, " \n");
					key = atoll(tok);
					ret = find(table, key);
					if (ret == NULL)
						printf("Not Exists\n");
					else
						printf("Key: %" PRId64 ", Value: %s\n", key, ret);
					free(ret); 
				}                          
				break;
			 case 'i': // insert
				if (fgets(buf, 255, stdin) != NULL) {
					tok = strtok(buf, " \n");
					table = atoi(tok);
					tok = strtok(NULL, " \n");
					key = atoll(tok);
					tok = strtok(NULL, " \n");
					insert(table, key, tok);
				}              
				break; 
			case 'j': // insert
				if (fgets(buf, 255, stdin) != NULL) {
					tok = strtok(buf, " \n");
					table = atoi(tok);
					tok = strtok(NULL, " \n");
					key = atoll(tok);
					tok = strtok(NULL, " \n");
					join_table(table, key, tok);
					printf("complete\n");
				}              
				break;     
 
			case 'd':
				if (fgets(buf, 30, stdin) != NULL) {
					tok = strtok(buf, " \n");
					table = atoi(tok);
					tok = strtok(NULL, " \n");
					key = atoll(tok);
					delete(table, key);
				}
				break;
			case 'c':
				if (fgets(buf, 30, stdin) != NULL) {
					tok = strtok(buf, " \n");
					table = atoi(tok);
					close_table(table);
				}
			   
				break;
		   
 
			default:
				fprintf(stderr, "SHUTDOWN\n");
				shutdown_db();
				return 0;
		}
	}
 
	return 0;

}