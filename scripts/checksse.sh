#!/bin/bash

cd ..

CC=`cat scripts/.buildinfo | grep CC | sed -e 's/^.*\=//'`

echo "#include <stdio.h>" > scripts/ssetest.c || { exit 1; }
echo 'int main() {float a = 1;float b = 2;printf("%f\n",a+b);return 0;}' >> scripts/ssetest.c

$CC -o scripts/ssetest -msse -msse2 -mfpmath=both scripts/ssetest.c &> /dev/null

scripts/ssetest &> /dev/null || { rm -f scripts/ssetest.c; rm -rf scripts/ssetest; echo ""; exit 0; }

echo " -msse -msse2 -ftree-vectorize -mfpmath=both "

exit 0;
