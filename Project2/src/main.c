#include "page.h"
#include <time.h>
#include <ctype.h>

int main(int argc, char ** argv) {
	int64_t key, temp, t;
	char instruction;
	char value[300];
	off_t testoffset;
	page *tpage;
	bool automate = true;
	// add_free_page(testoffset);
	srand(time(NULL));
	if (argc == 1)
		temp = open_db("./mydb");
	else {
		temp = open_db(argv[1]);
	}

	if (headerP->num_pages == 1 && automate) {
		int inputnum;
		char c = 'i';
		int temp, toWrite;
		printf("inputnum: ");
		scanf("%d", &inputnum);
		for (int i = inputnum; i >= 1; i--) {
			// t = rand() % 100000;
			t = i;
			sprintf(value, "%lld", t);
			printf("%lld %s\n", t, value);
			insert(t, value);
			find_and_print(t);
			printf("\n");
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
				printf("key: %lld\n\n", key);
				break;
			case 'f':
				scanf("%lld", &key);
				while(getchar() != '\n');
				find_and_print(key);
				break;
			case 't':
				print_tree();
				break;
			case 'd':scanf("%lld", &key);
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