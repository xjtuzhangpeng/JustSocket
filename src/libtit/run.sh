#!/bin/sh
gcc -shared -fPIC -c tit.c
gcc -shared -fPIC -o libtit.so tit.o
cp libtit.so ../../lib
