#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <LzmaLib.h>

#define LZMA_DIC_MIN (1 << 12)
#define LZMA_PROPS_SIZE 13	//The correct size

struct comprProps {
	unsigned dictSize;
	int level, numThreads, lc, lp, pb, fb;
};
int readHeader(unsigned char *data, size_t *dstLen, struct comprProps *p);

void setLzmaHeader(unsigned char *LzData, size_t srcLen, struct comprProps *p)
{
	int i;

	LzData[1] = (p->dictSize & 0xff000000UL) >> 24;
	LzData[2] = (p->dictSize & 0x00ff0000UL) >> 16;
	LzData[3] = (p->dictSize & 0x0000ff00UL) >> 8;
	LzData[4] = (p->dictSize & 0x000000ffUL) >> 0;
	for (i = 5; i < 13; i++) {
		LzData[i] = (unsigned char)srcLen;
		srcLen = srcLen >> 8;
	}
}

unsigned char *compressdBuf(char *src, size_t srcLen, int *res, size_t *bufLen,
			    struct comprProps *p)
{
	size_t propsSize;
	unsigned char *dest;

	propsSize = LZMA_PROPS_SIZE;
	*bufLen = srcLen + srcLen / 3 + 128;
	dest = (unsigned char *)malloc(propsSize + *bufLen);
	*res =
	    LzmaCompress((unsigned char *)(dest + LZMA_PROPS_SIZE), bufLen,
			 (unsigned char *)src, srcLen, dest, &propsSize,
			 p->level, p->dictSize, p->lc, p->lp, p->pb, p->fb,
			 p->numThreads);

	*bufLen = *bufLen + LZMA_PROPS_SIZE;
	setLzmaHeader(dest, srcLen, p);

	return dest;
}

unsigned char *expandBuf(unsigned char *src, size_t *srcLen, size_t *destSize,
			 int *res)
{;
	unsigned char *outBuf;

	outBuf = (unsigned char *)malloc(*destSize);
	*srcLen = *srcLen - LZMA_PROPS_SIZE;
	*res = LzmaUncompress(outBuf, destSize,
			      (unsigned char *)(src + LZMA_PROPS_SIZE), srcLen,
			      src, LZMA_PROPS_SIZE);

	return outBuf;
}

unsigned char *file2Buf(char *fName, size_t *size)
{
	FILE *file;
	unsigned char *cont;

	file = fopen(fName, "rb");
	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);
	cont = (unsigned char *)calloc(*size, sizeof(unsigned char));
	fread(cont, sizeof(char), *size, file);
	fclose(file);

	return cont;
}

void buf2File(char *fName, size_t size, unsigned char *buf)
{
	FILE *file;

	file = fopen(fName, "wb");
	fwrite(buf, sizeof(buf[0]), size, file);
	fclose(file);
}

int compressFile(char *inFile, char *outFile, struct comprProps *p)
{
	char *inBuf;
	unsigned char *outBuf;
	size_t bufSize, outLen;
	int res;

	inBuf = (char *)file2Buf(inFile, &bufSize);
	outBuf = compressdBuf(inBuf, bufSize, &res, &outLen, p);
	strcpy(outFile, inFile);
	strcat(outFile, ".lzma2");

	buf2File(outFile, outLen, outBuf);
	free(inBuf);
	free(outBuf);

	return res;
}

int expandFile(char *inFile, char *outFile, struct comprProps *p)
{
	unsigned char *inBuf, *outBuf;
	size_t fSize, outLen;
	FILE *file;
	int res;

	file = fopen(outFile, "wb");
	inBuf = file2Buf(inFile, &fSize);
	readHeader(inBuf, &outLen, p);
	outBuf = expandBuf(inBuf, &fSize, &outLen, &res);
	fwrite(outBuf, sizeof(char), outLen, file);
	fclose(file);
	free(inBuf);
	free(outBuf);

	return res;
}

int readHeader(unsigned char *data, size_t *dstLen, struct comprProps *p)
{
	unsigned int d;
	int i;

	d = data[0];
	if (d >= (9 * 5 * 5))
		return -1;
	p->lc = d % 9;
	d = d / 9;
	p->pb = d / 5;
	p->lp = d % 5;
	p->dictSize = 0;
	for (int i = 0; i < 4; i++)
		p->dictSize = (UInt32) data[i + 1] << (8 * i);
	if (p->dictSize < LZMA_DIC_MIN)
		p->dictSize = LZMA_DIC_MIN;
	*dstLen = 0;
	for (i = 5; i < 13; i++) {
		*dstLen =
		    ((uint64_t) (((uint8_t *) (data))[i]) << 8 *
		     (i - 5)) + *dstLen;
	}
	printf
	    ("LC: %d, PB: %d, LP: %d, Dictionary Size: %d KB, Uncompressed Size: %ld KB\n",
	     p->lc, p->pb, p->lp, p->dictSize / 1024, *dstLen / 1024);

	return 0;
}

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
