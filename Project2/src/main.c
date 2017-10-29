#include "page.h"

int main(int argc, char ** argv) {
	int64_t key, temp;
	char instruction;
	char value[300];
	off_t testoffset;
	page *tpage;
	printf("headerP size: %llu\n", sizeof(header_page));
	printf("Page size: %llu\n", sizeof(page));
	if (argc == 1)
		temp = open_db("./mydb");
	else {
		temp = open_db(argv[1]);
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