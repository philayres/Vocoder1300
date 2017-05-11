#!/bin/bash
# build.sh
objects=""

for filename in $( ls src/*.c ); do
f=$(echo $filename | cut -f 1 -d '.')
gcc -Iheader -o $f.o -c $f.c
objects=src/$f.o $objects
done


gcc -Iheader -shared -o libmylib.so $objects -lm 

export LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH
cd test
gcc -O2 c2enc.c -o c2enc -lcodec1300 -lm 
gcc -O2 c2dec.c -o c2dec -lcodec1300 -lm