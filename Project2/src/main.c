#include "page.h"
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

int main(int argc, char ** argv) {
	int64_t key, temp, t;
	char instruction;
	char value[300];
	off_t testoffset;
	page *tpage;
	int automate = 1;
	// add_free_page(testoffset);
	srand(time(NULL));
	if (argc == 1)
		temp = open_db("./mydb");
	else {
		temp = open_db(argv[1]);
	}
	printf("automate db?(Y:1, N:0): ");
	scanf("%d", &automate);
	//2^20 == 1048576 == (sizeof_db = 4GB)
	if (automate) {
		bool check[1048576];
		bool nodup;
		int inputnum, deletenum;
		char c = 'i';
		int temp, toWrite;

		printf("insert num: ");
		scanf("%d", &inputnum);
		memset(check, false, sizeof(check));
		for (int i = inputnum; i >= 1; i--) {
			do {
				nodup = true;
				t = rand() % inputnum;
				if (check[t])
					nodup = false;
			}	while(!nodup);
			check[t] = true;
			// t = i;
			sprintf(value, "%lld", t);
			// printf("%lld %s\n", t, value);
			insert(t, value);
		}

		printf("delete num: ");
		scanf("%d", &deletenum);
		memset(check, false, sizeof(check));
		for (int i = deletenum; i >= 1; i--) {
			do {
				nodup = true;
				t = rand() % deletenum;
				if (check[t])
					nodup = false;
			}	while(!nodup);
			check[t] = true;
			// t = i;
			printf("delecting %lld\n", t);
			if (delete(t) == -1) {
				print_tree();
				exit(EXIT_FAILURE);
			}
		}
	}
	while(1) {
		printf("i / f / t / d / q\n");
		do 
			instruction = getchar();
		while(isspace(instruction));
		printf("instruction: %c\n", instruction);
		switch(instruction) {
			case 'i':
				scanf("%lld %s", &key, value);
				while(getchar() != '\n');
				insert(key, value);
				// printf("key: %lld\n\n", key);
				break;
			case 'f':
				scanf("%lld", &key);
				while(getchar() != '\n');
				find_and_print(key);
				break;
			case 't':
				print_tree();
				break;
			case 'd':
				scanf("%lld", &key);
				if (delete(key) != -1)
					printf("key: %lld deleted\n\n", key);
				break;
			case 'q':
				return EXIT_SUCCESS;
			default:
				while(getchar() != '\n');
				printf("No such Instruction\n");
		}
	}
	return EXIT_SUCCESS;
}