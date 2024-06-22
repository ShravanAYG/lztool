#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <LzmaLib.h>
#include "lztool.h"

void
setCompressProps(int level, unsigned dictSize, int lc, int lp, int pb,
		 int fb, int numThreads, struct comprProps *p)
{
	p->level = level;
	p->dictSize = dictSize;
	p->lc = lc;
	p->lp = lp;
	p->pb = pb;
	p->fb = fb;
	p->numThreads = numThreads;
}

int main()
{
	printf("LZMA2 compress test by Shravan A Y\n\n");

	int sel, errno;
	char inFile[256], outFile[256];
	struct comprProps props, pr2;

	setCompressProps(5, 4, 3, 0, 2, 32, 1, &props);
	errno = 0;
	printf("1) Compress\n2) Decompress\nYour Option: ");
	fscanf(stdin, "%d", &sel);
	switch (sel) {
	case 1:
		printf("Enter file name: ");
		fscanf(stdin, "%s", inFile);
		if (compressFile(inFile, outFile, &props) != SZ_OK)
			printf("ERROR%d: Compression failed!", errno++);
		break;
	case 2:
		printf("Enter lzma2 file name: ");
		fscanf(stdin, "%s", inFile);
		printf("Write output to ");
		strcpy(outFile, inFile);
		outFile[strlen(inFile) - 6] = '\0';
		printf("%s ? (yes = 1, no = 2): ", outFile);
		fscanf(stdin, "%d", &sel);
		if (sel == 1)
			expandFile(inFile, outFile, &pr2);
		else if (sel == 2) {
			printf("Write output to: ");
			fscanf(stdin, "%s", outFile);
			expandFile(inFile, outFile, &pr2);
		} else
			printf("ERROR%d: Unknown option\n", errno++);
		break;
	default:
		printf("ERROR%d: Unknown option\n", errno++);
		break;
	}
	if (errno == 0)
		return 0;
	printf("Program exited with %d errors\n", errno);
	return 1;
}
