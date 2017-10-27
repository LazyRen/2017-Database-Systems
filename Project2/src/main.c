#include "page.h"

int main(int argc, char ** argv) {
	int a;

	a = open_db("/Users/LazyRen/Documents/Programming/TEST/text");
	if (a != 0)
		printf("failed\n");
	return EXIT_SUCCESS;
}