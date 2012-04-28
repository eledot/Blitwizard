
# Working dir of this should be tests/

# This gathers some variables and decides on how to run things

# Gather important variables about our build environment:
previousdir=`pwd`
cd ../
CC=`cat scripts/.buildinfo | grep CC | sed -e 's/^.*\=//'`
CXX=`cat scripts/.buildinfo | grep CXX | sed -e 's/^.*\=//'`
HOST=`cat scripts/.buildinfo | grep HOST | sed -e 's/^.*\=//'`
AR=`cat scripts/.buildinfo | grep AR | sed -e 's/^.*\=//'`
EXEEXT=`cat scripts/.buildinfo | grep EXEEXT | sed -e 's/^.*\=//'`
cd $previousdir

# Find out how to run blitwizard:
RUNBLITWIZARD="../bin/blitwizard$EXEEXT"
RUNBINARY=""
windowscheck=`echo $HOST | grep mingw`
if [ -n "$windowscheck" ]; then
    RUNBINARY="wine "
    RUNBLITWIZARD="wine ../bin/blitwizard$EXEEXT"
fi

# Emit a warning if in a cross compilation environment
if [ -n "$HOST" ]; then
    echo " ** TEST WARNING: You are cross-compiling. Testing might not work."
fi


