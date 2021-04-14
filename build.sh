#!/bin/sh

ARG=""

for F in $(find src/ -name "*.c" -type f); do
	ARG="${ARG} ${F}"
done;


gcc $ARG -o ftp.out -static -O0
