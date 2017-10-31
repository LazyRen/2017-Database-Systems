#include "page.h"

int main(int argc, char ** argv)
{
	char path[100];
	int temp = -1;
	while (temp != 0) {
		if (argc >= 2)
			temp = open_db(argv[1]);
		else {
			printf("input path of db: ");
			scanf("%s", path);
			temp = open_db(path);
		}	
	}
	print_tree();
}