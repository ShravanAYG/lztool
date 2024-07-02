#!/bin/sh
if sha256sum -c *.sha256sum
then
	echo Files are correct! Checksum Matched!
else
	echo Check failed!
fi

