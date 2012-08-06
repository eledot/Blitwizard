#!/bin/bash

# This test confirms os.ls() (a blitwizard lua api function) works.
# It lists the files in the templates/ directory using blitwizard/os.ls,
# then does the same in bash and compares the results.

source preparetest.sh

# Get output from blitwizard
echo "
-- create a static and a movable object:
local obj1 = blitwiz.physics2d.createStaticObject()
local obj2 = blitwiz.physics2d.createMovableObject()
blitwiz.physics2d.setShapeOval(obj1, 3, 2)
blitwiz.physics2d.setShapeRectangle(obj2, 5, 4)

-- a ray is also generating new references and affecting garbage collection:
blitwiz.physics2d.ray(-10, 0, 0, 0)

-- collect garbage until something happens
print(\"Physics GC test phase 1/2\")
local i = 1
while i < 1000 do
    collectgarbage()
    i = i + 1
end

-- Now nil the references explicitely:
obj1 = nil
obj2 = nil

-- collect more garbage
print(\"Physics GC test phase 2/2\")
local i = 1
while i < 1000 do
    collectgarbage()
    i = i + 1
end
os.exit(0)
" > ./test.lua
$RUNBLITWIZARD ./test.lua
RETURNVALUE="$?"
rm ./test.lua

if [ "x$RETURNVALUE" = "x0" ]; then
    exit 0
else
    exit 1
fi


