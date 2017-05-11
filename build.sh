#!/bin/bash
# build.sh
for f in $( ls src ); do
gcc -o $f.o -c $f.c
done
