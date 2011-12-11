#!/bin/bash

cd ..

CC=`cat scripts/.buildinfo | grep CC | sed -e 's/^.*\=//'`
EXEEXT=`cat scripts/.buildinfo | grep EXEEXT | sed -e 's/^.*\=//'`

echo "#include <stdio.h>" > scripts/ssetest.c || { exit 1; }
echo 'int main() {float a = 1;float b = 2;printf("%f\n",a+b);return 0;}' >> scripts/ssetest.c

$CC -o scripts/ssetest$EXEEXT -msse -msse2 -mfpmath=both scripts/ssetest.c &> /dev/null

WINECMD=""

if [ -n "$EXEEXT" ]; then
	WINECMD="wine "
#	export WINEDEBUG=-all
fi

scripts/ssetest$EXEEXT &> /dev/null || {
	${WINECMD}scripts/ssetest$EXEEXT &> /dev/null || {rm -f scripts/ssetest.c; rm -rf scripts/ssetest$EXEEXT; exit 0;}
}

echo " -msse -msse2 -mfpmath=both "

rm scripts/ssetest.c
rm scripts/ssetest$EXEEXT

exit 0;
