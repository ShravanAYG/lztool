#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <LzmaLib.h>
#include "lztool.h"

void printHelp(char *pName)
{
	printf("Usage: %s [OPTION]... [FILE]...\n\
Compress or decompress FILEs in the .lzma2 format.\n\n\
  -z, --compress\tforce compression\n\
  -d, --decompress\tforce decompression\n\
  -t, --test\t\ttest compressed file integrity\n\
  -l, --list\t\tlist information about .xz files\n\
  -k, --keep\t\tkeep (don't delete) input files\n\
  -f, --force\t\tforce overwrite of output file and (de)compress links\n\
  -c, --stdout\t\twrite to standard output and don't delete input files\n\
  -0 ... -9\t\tcompression preset; default is 6; take compressor *and*\n\
\t\t\tdecompressor memory usage into account before using 7-9!\n\
  -e, --extreme\t\ttry to improve compression ratio by using more CPU time;\n\
\t\t\tdoes not affect decompressor memory requirements\n\
  -T, --threads=NUM\tuse at most NUM threads; the default is 1; set to 0\n\
\t\t\tto use as many threads as there are processor cores\n\
  -q, --quiet\t\tsuppress warnings; specify twice to suppress errors too\n\
  -v, --verbose\t\tbe verbose; specify twice for even more verbose\n\
  -h, --help\t\tdisplay this short help and exit\n\
  -H, --long-help\tdisplay the long help (lists also the advanced options)\n\
  -V, --version\t\tdisplay the version number and exit\n\n\
With no FILE, or when FILE is -, read standard input.\n\n\
Report bugs to <shravanay205@gmail.com>.\n\
lztools home page: <https://github.com/ShravanAYG/lztool>\n", pName);
}

int main(int argc, char *argv[])
{
	int err_no = 0, keep = 1, level = 5;
	char outFile[256], inFile[256];
	struct comprProps props, outprops;

	setCompressProps(5, 4, 3, 0, 2, 32, 1, &props);
	err_no = 0;

	for (int i = 1; i < argc && err_no == 0; i++) {
		if (!strcmp(argv[i], "--compress") || !strcmp(argv[i], "-z")) {
			printf("Compressing using level %d\n", props.level);
			if (i + 1 < argc) {
				if (compressFile(argv[i + 1], outFile, &props)
				    != SZ_OK)
					printf("ERROR%d: Compression failed!\n",
					       err_no++);
				i++;
				strcpy(inFile, argv[i]);
				keep = 0;
			}

		} else if (!strcmp(argv[i], "--decompress")
			   || !strcmp(argv[i], "-d")) {
			if (i + 1 < argc) {
				strcpy(outFile, argv[i + 1]);
				outFile[strlen(argv[i + 1]) - 6] = '\0';
				if (!fileExists(outFile)) {
					if (expandFile
					    (argv[i + 1], outFile, &outprops)
					    != SZ_OK)
						printf
						    ("ERROR%d: decompression failed!",
						     err_no++);
					i++;
					strcpy(inFile, argv[i]);
					keep = 0;
				} else
					printf
					    ("ERROR%d: File %s alredy exists\n",
					     err_no++, outFile);
			}
		} else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
			printHelp(argv[0]);
		} else if (!strcmp(argv[i], "--keep") || !strcmp(argv[i], "-k")) {
			keep = 1;
		} else if (*argv[i] == '-') {
			level = argv[i][1] - '0';
			if (level >= 0 && level <= 9)
				props.level = level;
		} else
			printf("ERROR%d: Unknown option\n", err_no++);

	}

	if (argc == 1) {
		if (compressFile("/dev/stdin", "stdin.lzma2", &props)
		    != SZ_OK)
			printf("ERROR%d: Compression failed!\n", err_no++);
	}
	if (keep == 0 && err_no == 0)
		remove(inFile);

	if (err_no == 0)
		return 0;
	printf("Program exited with %d errors\n", err_no);
	return 1;
}
