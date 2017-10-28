#include "page.h"

int main(int argc, char ** argv) {
	int64_t key, temp;
	char instruction;
	char *input_file;
	char value[300];

	if (argc == 1) {
	temp = open_db("./mydb");
	if (temp != 0)
		printf("failed\n");
	} else {
		temp = open_db(input_file);
	if (temp != 0)
		printf("failed\n");
	}
	while(1) {
		scanf("%c", &instruction);
		switch(instruction) {
			case 'i':
				scanf("%lld %s", &key, value);
				while(getchar() != '\n');
				insert(key, value);
				printf("insertiong finished\n");
				break;
			case 'f':
				scanf("%lld", &key);
				while(getchar() != '\n');
				find_and_print(key);
				break;
			case 'q':
				return EXIT_SUCCESS;
			default:
				printf("No such Instruction\n");
		}
	}
	return EXIT_SUCCESS;
}