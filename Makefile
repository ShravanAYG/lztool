LZMASDK_DIR="lzma2407"

all:
	gcc -c ${LZMASDK_DIR}/C/*.c -Wall
	gcc -c main.c -I${LZMASDK_DIR}/C
	gcc *.o -o lztool

clean: all
	rm *.o

