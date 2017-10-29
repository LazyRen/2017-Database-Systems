#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main()
{
	FILE *fp;
	int inputnum = 32;
	char c;
	int len, temp, toWrite;

	srand(time(NULL));
	fp = fopen("./testinput", "w");
	for (int i = 1; i <= inputnum; i++) {
		temp = i;
		len = 2;
		fprintf(fp, "i %d %d:", i, i);
		// while(temp/10 != 0) {
		// 	temp = temp/10;
		// 	len++;
		// }
		// toWrite = 119 - len;
		// for (int j = 0; j < toWrite; j++) {
		// 	c = rand() % 52 + 65;
		// 	fprintf(fp, "%c", c);
		// }
		fprintf(fp, "\n");
	}
}