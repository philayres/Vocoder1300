#!/bin/bash
 cd test
 rm hts1.c2 
 rm hts1.c2cb
 rm hts1-c2.raw 
 rm hts1-c2cb.raw
 rm hts1.wav
 rm hts1-c2.wav
 rm hts1-c2cb.wav
 
 if [ ! -e "LDC97S44.wav" ]
 then
   echo getting news
 #  wget https://catalog.ldc.upenn.edu/desc/addenda/LDC97S44.wav
   echo converting news to raw
   sox LDC97S44.wav -r 8000 LDC97S44-8k.wav
   sox LDC97S44-8k.wav -t raw LDC97S44-8k.raw
 fi
 
 
 
 echo encode decode
 ./c2enc hts1.raw hts1.c2 
 ./c2dec hts1.c2 hts1-c2.raw
 ./c2enc hts1.raw hts1.c2cb charbits
 ./c2dec hts1.c2cb hts1-c2cb.raw charbits
 
 ./c2enc LDC97S44-8k.raw LDC97S44-8k.c2cb charbits
 ./c2dec LDC97S44-8k.c2cb LDC97S44-8k-c2cb.raw charbits 
 
 xxd hts1-c2.raw hts1-c2.raw.hex
 xxd hts1-c2cb.raw hts1-c2cb.raw.hex
 
 diff hts1-c2.raw.hex hts1-c2cb.raw.hex | more
 
 sox -r 8000 -e unsigned -b 16 -c 1 hts1.raw hts1.wav
 sox -r 8000 -e unsigned -b 16 -c 1 hts1-c2.raw hts1-c2.wav
 sox -r 8000 -e unsigned -b 16 -c 1 hts1-c2cb.raw hts1-c2cb.wav
 sox -r 8000 -e unsigned -b 16 -c 1 LDC97S44-8k-c2cb.raw LDC97S44-8k-c2cb.wav