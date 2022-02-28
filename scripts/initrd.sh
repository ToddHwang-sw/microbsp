#!/bin/sh

EXTNAME=cpio.gz

case "$1" in
	"help")
		echo "Decompress/Compress current folder utility"
		echo "$0  compress    <filename>.cpio.gz"
		echo "$0  decompress  <filename>.cpio.gz"
		;;
	"compress")
		find . | cpio -o -c -R root:root | gzip -9 > $2.$EXTNAME
		;;
	"decompress")
		zcat $2.$EXTNAME  | cpio -idmv
		;;
esac

