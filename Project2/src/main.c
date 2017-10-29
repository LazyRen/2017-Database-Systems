#include "page.h"
#include <time.h>
int main(int argc, char ** argv) {
	int64_t key, temp;
	char instruction;
	char value[300];
	off_t testoffset;
	page *tpage;
	bool automate = true;

	if (argc == 1)
		temp = open_db("./mydb");
	else {
		temp = open_db(argv[1]);
	}

	if (headerP->num_pages == 1 && automate) {
		printf("hello\n");
		int inputnum;
		char c = 'i';
		int temp, toWrite;
		printf("inputnum: ");
		scanf("%d", &inputnum);
		for (int i = 1; i <= inputnum; i++) {
			sprintf(value, "%d", i);
			printf("%s\n", value);
			insert(i, value);
			find_and_print(i);
			printf("\n");
		}
	}
	while(1) {
		printf("i / f / q\n");
		instruction = fgetc(stdin);
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
			case 'q':
				return EXIT_SUCCESS;
			default:
				while(getchar() != '\n');
				printf("No such Instruction\n");
		}
	}
	return EXIT_SUCCESS;
}