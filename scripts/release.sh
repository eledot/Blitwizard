#!/bin/bash

if [ -z "$1" ]; then
    echo "You need to specify a release name, e.g. 1.0."
    exit
fi

echo "Please note this script should run on a linux machine with a proper cross-compiler to allow for the Windows compilation.";
echo "Set the cross-compiler in create-release-archive.sh before proceeding.";

echo "Ready to proceed? [y/N]";

read a
if [[ $a != "Y" && $a != "y" ]]; then
    echo "Aborted.";
	exit 0;
fi

echo "Creating linux archive."
sh ./create-release-archive.sh $1 || { echo "Linux release failed."; exit 1; }
sh ./create-release-archive.sh $2 linux-to-win || { echo "Windows release failed"; exit 1; }

