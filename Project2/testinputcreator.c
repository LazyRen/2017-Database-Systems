#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main()
{
	FILE *fp;
	int inputnum = 7750;
	char c;
	int len, temp, toWrite;

	srand(time(NULL));
	fp = fopen("./testinput", "w");
	for (int i = 1; i <= inputnum; i++) {
		temp = i;
		len = 2;
		fprintf(fp, "i %d %d\n", i, i);
		fprintf(fp, "f %d\n", i);
	}
}