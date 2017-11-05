#include "page.h"
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

int main(int argc, char ** argv) {
	// int64_t key, temp, t;
	// char instruction;
	// char value[300];
	// int automate = 1;
	// // add_free_page(testoffset);
	// srand(time(NULL));
	// if (argc == 1)
	// 	temp = open_db("./mydb");
	// else {
	// 	temp = open_db(argv[1]);
	// }
	// printf("automate db?(Y:1, N:0): ");
	// scanf("%d", &automate);
	// //2^20 == 1048576 == (sizeof_db = 4GB)
	// if (automate) {
	// 	bool check[1048576];
	// 	bool nodup;
	// 	int64_t inputnum, deletenum;

	// 	printf("insert num: ");
	// 	scanf("%"PRId64, &inputnum);
	// 	memset(check, false, sizeof(check));
	// 	for (int i = inputnum; i >= 1; i--) {
	// 		do {
	// 			nodup = true;
	// 			t = rand() % inputnum;
	// 			if (check[t])
	// 				nodup = false;
	// 		}	while(!nodup);
	// 		check[t] = true;
	// 		// t = i;
	// 		sprintf(value, "%"PRId64, t);
	// 		// printf("%lld %s\n", t, value);
	// 		insert(t, value);
	// 	}

	// 	printf("delete num: ");
	// 	scanf("%"PRId64, &deletenum);
	// 	memset(check, false, sizeof(check));
	// 	for (int i = deletenum; i >= 1; i--) {
	// 		do {
	// 			nodup = true;
	// 			t = rand() % deletenum;
	// 			if (check[t])
	// 				nodup = false;
	// 		}	while(!nodup);
	// 		check[t] = true;
	// 		// t = i;
	// 		// printf("delecting %l"PRId64"\n", t);
	// 		delete(t);
	// 		// if (delete(t) == -1) {
	// 		// 	print_tree();
	// 		// 	exit(EXIT_FAILURE);
	// 		// }
	// 	}
	// }
	// while(1) {
	// 	printf("i / f / t / d / q\n");
	// 	do 
	// 		instruction = getchar();
	// 	while(isspace(instruction));
	// 	printf("instruction: %c\n", instruction);
	// 	switch(instruction) {
	// 		case 'i':
	// 			scanf("%"PRId64" %s", &key, value);
	// 			while(getchar() != '\n');
	// 			insert(key, value);
	// 			// printf("key: %lld\n\n", key);
	// 			break;
	// 		case 'f':
	// 			scanf("%"PRId64, &key);
	// 			while(getchar() != '\n');
	// 			find_and_print(key);
	// 			break;
	// 		case 't':
	// 			print_tree();
	// 			break;
	// 		case 'd':
	// 			scanf("%"PRId64, &key);
	// 			if (delete(key) != -1)
	// 				printf("key: %"PRId64" deleted\n\n", key);
	// 			break;
	// 		case 'q':
	// 			return EXIT_SUCCESS;
	// 		default:
	// 			while(getchar() != '\n');
	// 			printf("No such Instruction\n");
	// 	}
	// }
	// return EXIT_SUCCESS;


	int64_t input;
	char instruction;
	char *buf;
	buf = malloc(120);
	open_db("test.db");
	while(scanf("%c", &instruction) != EOF){
		switch(instruction){
			case 'i':
				scanf("%"PRId64" %s", &input, buf);
				insert(input, buf);
				break;
			case 'f':
				scanf("%"PRId64, &input);
				buf = find(input);
				if (buf) {
					printf("Key: %"PRId64", Value: %s\n", input, buf);
				} else
					printf("Not Exists\n");

				fflush(stdout);
				break;
			case 'd':
				scanf("%"PRId64, &input);
				delete(input);
				break;
			case 'q':
				while(getchar() != (int)'\n');
				return EXIT_SUCCESS;
				break;
		}
		while (getchar() != (int)'\n');
	}
	printf("\n");
	return EXIT_SUCCESS;
}