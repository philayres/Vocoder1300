#!/bin/bash
# build.sh
objects=""
rm src/*.o
rm encode/*.o
rm decode/*.o
rm *.so
rm test/c2enc
rm test/c2dec

for filename in $( ls src/*.c encode/*.c decode/*.c ); do
f=$(echo $filename | cut -f 1 -d '.')
echo compiling $f.c to $f.o
gcc -Iheader -fPIC -fstack-protector-all -o $f.o -c $f.c -g
objects="$objects $f.o"

done

echo making libcodec1300.so from $objects
gcc -Iheader -shared -o libcodec1300.so $objects -lm -g

gcc -O2 test/c2enc.c -o test/c2enc -lcodec1300 -lm -L. -g -fstack-protector-all
gcc -O2 test/c2dec.c -o test/c2dec -lcodec1300 -lm -L. -g -fstack-protector-all
chmod 775 test/c2enc
chmod 775 test/c2dec

sudo cp libcodec1300.so /usr/local/lib
sudo ldconfig
