#!/bin/bash

cd ..

if [ ! -f "scripts/.buildinfo" ]; then
	exit 1
fi

CC=`cat scripts/.buildinfo | grep CC | sed -e 's/^.*\=//'`
EXEEXT=`cat scripts/.buildinfo | grep EXEEXT | sed -e 's/^.*\=//'`

echo "#include <stdio.h>" > scripts/ssetest.c || { exit 1; }
echo 'int main() {float a = 1;float b = 2;printf("%f\n",a+b);return 0;}' >> scripts/ssetest.c

errorhappened="no"
$CC -o scripts/ssetest$EXEEXT -msse -msse2 -mfpmath=both scripts/ssetest.c > /dev/null 2>&1 || { errorhappened="yes"; }

WINECMD=""

if [ -n "$EXEEXT" ]; then
	WINECMD="wine "
#	export WINEDEBUG=-all
fi

if [ "$errorhappened" = "no" ]; then
	scripts/ssetest$EXEEXT > /dev/null 2>&1 || {
		${WINECMD}scripts/ssetest$EXEEXT > /dev/null 2>&1 || { errorhappened="yes"; }
	}
fi

if [ "$errorhappened" = "yes" ]; then
	rm -f scripts/ssetest.c
	rm -f scripts/ssetest$EXEEXT
	exit 0
fi

echo " -msse -msse2 -mfpmath=both "

rm scripts/ssetest.c
rm scripts/ssetest$EXEEXT

exit 0
