#### vocoder1300
A codec2 LPC-10 1300 bps Speech Vocoder

The purpose of this version of the vocoder, is to break out the encoder and decoder parts into separate directories, and to delete the other modes which are all deprecated anyway. No one uses anything besides 1300 and 700B.

The 700B vocoder is in another repository, as that is used with the coherent modem.

The files were grouped using Netbeans, and compiled in Release mode. For the Ubuntu 64-bit I got the following size for the dynamic library:
```
   text	   data	    bss	    dec	    hex	filename
  30858	   1416	  11984	  44258	   ace2	libcodec1300.so
```
For Raspberry Pi 2 B+ I get:
```
   text	   data	    bss	    dec	    hex	filename
  23238	    760	  11780	  35778	   8bc2	libcodec1300.so
```
If you burst the ZIP file it will create all the directories, and you can just type ```make``` and it will build the library. I used -O2 for optimization.

#### Design Change
I'm an old geezer, so I like to see my data in the BSS section, rather then allocated into RAM using malloc. So you will notice that right off the bat, as there is an API change where you don't have to worry about the malloc pointer.

#### Testing
Here's how I compile the test programs. Put the library file in ```/usr/local/lib``` and do a ```sudo ldconfig```:
```
  cc -O2 c2enc.c -o c2enc -lcodec1300 -lm
  cc -O2 c2dec.c -o c2dec -lcodec1300 -lm
```
Then you can run the test programs (the binaries in the repository are for Ubuntu 64-bit):
```
./c2enc hts1.raw hts1.c2
./c2dec hts1.c2 result.raw
sox -r 8000 -c1 -e signed -b 16 result.raw test.wav
play test.wav
```
You can also use the original c2enc/c2dec in the codec2, to compare results. In this case you can:
```
xxd result.raw >my-result.hex
xxd original.raw >original.hex
diff original.hex my-result.hex
```
When the two are exactly the same, I will declare victory...

As it stands right now, the audio sounds the same as codec2, but the RAW output from ```c2dec``` has some different bytes. Just a few bytes here and there. They seem to be off by 1 when they are wrong.
