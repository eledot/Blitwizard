#!/bin/bash

# This test confirms os.ls() (a blitwizard lua api function) works.
# It lists the files in the templates/ directory using blitwizard/os.ls,
# then does the same in bash and compares the results.

source preparetest.sh

# Get output from blitwizard
echo "local outputline = \"\"
os.chdir(\"../templates/\")
filelist = {}

-- get all files (unsorted)
for index,file in ipairs(os.ls(\"\")) do
    filelist[#filelist+1] = file
end

-- sort files and turn into string:
table.sort(filelist)
for index,file in ipairs(filelist) do
    if #outputline > 0 then
        outputline = outputline .. \" \"
    end
    outputline = outputline .. file
end

print(\"xx\" .. outputline .. \"yy\")" > ./test.lua
$RUNBLITWIZARD ./test.lua > ./testoutput

# Get comparison output through bash
olddir=`pwd`
cd ../templates/
bashresult=""
for f in *
do
    if [ -n "$bashresult" ]; then
        bashresult="$bashresult ";
    fi
    bashresult="$bashresult$f"
done
cd $olddir

testoutput="`cat ./testoutput | grep xx | sed 's/[ \n\r]*$//g'`"
bashoutput="xx${bashresult}yy"

rm ./testoutput

if [ "x$testoutput" = "x$bashoutput" ]; then
    exit 0
else
    exit 1
fi


