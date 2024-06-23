# lztool
Simple tool to compress/expand files using LibLzma/LzmaSDK

# IMPORTANT NOTE
This program will <b>NOT WORK</b> with the ".lzma" file produces by lzmautils program, that file format is actually <b>XZ</b> compressed. Please use only ".lzma2" or equivalent files as it will contain the proper file format (with 13 byte header - see ./lzma2402/DOC/lzma_specification.txt).

# Using
Compile and run using ./lztool.
Multiple files or directories can be compressed by creating a `tar` archieve and then using lzma2 compressor.

# Examples
`$ ./lztool --decompress ./test/tmp.bin.lzma2 --keep`

# Screenshot
<image src="test/Screenshot.png"/>
