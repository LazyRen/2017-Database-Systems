#include "page.h"

int main(int argc, char ** argv)
{
	char *path;
	path = (char*)malloc(100);
	int temp = -1;
	if (sizeof(header_page) != 4096 || sizeof(page) != 4096)
	{
		printf("header_page or page's size is not 4096\n");
		exit(EXIT_FAILURE);
	}

	while (temp != 0) {
		if (argc >= 2) {
			temp = open_db(argv[1]);
			argc = 1;
		}
		else {
			printf("Give DB file location as an second argument in terminal\n");
		}	
	}
	print_tree();
}