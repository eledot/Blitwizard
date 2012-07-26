#!/bin/bash

# This test confirms os.ls() (a blitwizard lua api function) works.
# It lists the files in the templates/ directory using blitwizard/os.ls,
# then does the same in bash and compares the results.

source preparetest.sh

# Get output from blitwizard
echo "
-- create a static and a movable object:
local obj1 = blitwiz.physics.createStaticObject()
local obj2 = blitwiz.physics.createMovableObject()
blitwiz.physics.setShapeOval(obj1, 3, 2)
blitwiz.physics.setShapeRectangle(obj2, 5, 4)

-- a ray is also generating new references and affecting garbage collection:
blitwiz.physics.ray(-10, 0, 0, 0)

-- collect garbage until something happens
local i = 1
while i < 1000 do
    collectgarbage()
    i = i + 1
end
os.exit(0)
" > ./test.lua
$RUNBLITWIZARD ./test.lua

if [ "x$?" = "x0" ]; then
    exit 0
else
    exit 1
fi


