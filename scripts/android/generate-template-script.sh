#!/bin/sh

FILES=blitwizard-android/assets/templates/*
for f in $FILES
do
	PTH=`echo "$f" | sed "s/blitwizard-android\\/assets\\///g"`
	FILENAME=`echo "$f" | sed "s/.*\\///g"`
	if [ -d "$f" ]; then
		echo "dofile(\"$PTH/$FILENAME.lua\")"
	fi
done

