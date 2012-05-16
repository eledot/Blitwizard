#!/bin/bash

# This script is called from src/Makefile.am to copy the resulting binary to bin/
# The working dir is src/

luatarget=`cat ../scripts/.buildinfo | grep luatarget | sed -e 's/^luatarget\=//'`

# Copy the bin which has .exe or not, depending on our platform
if [ "x$luatarget" = xmingw ]; then
    cp ./blitwizard.exe ../bin || { echo "Failed to copy binary."; exit 1; }
else
    cp ./blitwizard ../bin || { echo "Failed to copy binary."; exit 1; }
fi

