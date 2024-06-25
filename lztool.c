#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <LzmaLib.h>
#include "lztool.h"

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

	propsSize = LZMA2_PROPS_SIZE;
	*bufLen = srcLen + srcLen / 3 + 128;
	dest = (unsigned char *)malloc(propsSize + *bufLen);
	*res =
	    LzmaCompress((unsigned char *)(dest + LZMA2_PROPS_SIZE), bufLen,
			 (unsigned char *)src, srcLen, dest, &propsSize,
			 p->level, p->dictSize, p->lc, p->lp, p->pb, p->fb,
			 p->numThreads);

	*bufLen = *bufLen + LZMA2_PROPS_SIZE;
	setLzmaHeader(dest, srcLen, p);

	return dest;
}

unsigned char *expandBuf(unsigned char *src, size_t *srcLen, size_t *destSize,
			 int *res)
{;
	unsigned char *outBuf;

	outBuf = (unsigned char *)malloc(*destSize);
	*srcLen = *srcLen - LZMA2_PROPS_SIZE;
	*res = LzmaUncompress(outBuf, destSize,
			      (unsigned char *)(src + LZMA2_PROPS_SIZE), srcLen,
			      src, LZMA2_PROPS_SIZE);

	return outBuf;
}

struct fnode {
	char *cont;
	size_t size;
	struct fnode *next;
};

char *largeFile(FILE * file, size_t *bsize)
{
	size_t size;
	int nnode = 0, i = 0;
	struct fnode *s_ptr, *n_node;
	char tmp[65536], *buffer;

	s_ptr = n_node = malloc(sizeof(struct fnode));

	while (1) {
		size = fread(tmp, sizeof(char), 65536, file);
		if (size == 0)
			break;

		n_node->cont = (char *)calloc(size, sizeof(char));
		n_node->size = size;
		memcpy(n_node->cont, tmp, size);

		if (size == 65536) {
			n_node->next =
			    (struct fnode *)malloc(sizeof(struct fnode));
			n_node = n_node->next;
			nnode++;
		} else {
			n_node->next = NULL;
			break;
		}
	}

	*bsize = size + (nnode * 65536);
	buffer = (char *)calloc(*bsize, sizeof(char));

	n_node = s_ptr;
	while (n_node != NULL) {
		memcpy(&buffer[i * 65536], n_node->cont, n_node->size);
		struct fnode *temp = n_node;
		n_node = n_node->next;
		free(temp->cont);
		free(temp);
		i++;
	}

	return buffer;
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

	if (!strcmp(inFile, "/dev/stdin")) {
		inBuf = largeFile(stdin, &bufSize);
	} else {
		inBuf = (char *)file2Buf(inFile, &bufSize);
		strcpy(outFile, inFile);
		strcat(outFile, ".lzma2");
	}
	outBuf = compressdBuf(inBuf, bufSize, &res, &outLen, p);
	if (!fileExists(outFile))
		buf2File(outFile, outLen, outBuf);
	else {
		printf("Compressed file %s alredy available\n", outFile);
		res = -1;
	}
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

int fileExists(char *fName)
{
	FILE *file;
	if ((file = fopen(fName, "r"))) {
		fclose(file);
		return 1;
	}
	return 0;
}
