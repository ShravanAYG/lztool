/* main.c */
#define LZMA_DIC_MIN (1 << 12)
#define LZMA2_PROPS_SIZE 13	//The correct size

struct comprProps {
	unsigned dictSize;
	int level, numThreads, lc, lp, pb, fb;
};

void setLzmaHeader(unsigned char *LzData, size_t srcLen,
		   struct comprProps *properties);
unsigned char *compressdBuf(char *src, size_t srcLen, int *result,
			    size_t *bufferLen, struct comprProps *properties);
unsigned char *expandBuf(unsigned char *src, size_t *srcLen, size_t *destSize,
			 int *result);
unsigned char *file2Buf(char *fileName, size_t *size);
void buf2File(char *fileName, size_t size, unsigned char *buffer);
int compressFile(char *inFile, char *outFile, struct comprProps *properties);
int expandFile(char *inFile, char *outFile, struct comprProps *properties);
int readHeader(unsigned char *LzData, size_t *destLen,
	       struct comprProps *properties);
void setCompressProps(int level, unsigned dictSize, int lc, int lp, int pb,
		      int fb, int numThreads, struct comprProps *properties);
char *largeFile(size_t *bufferSize, FILE * file);
int fileExists(char *fName);
