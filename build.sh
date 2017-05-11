#!/bin/bash
# build.sh
objects=""
cd src
for f in $( ls src ); do
gcc -o $f.o -c $f.c
objects=src/$f.o $objects
done

cd ..
gcc -shared -o libmylib.so $objects -lm 

export LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH
cd test
gcc -O2 c2enc.c -o c2enc -lcodec1300 -lm 
gcc -O2 c2dec.c -o c2dec -lcodec1300 -lm