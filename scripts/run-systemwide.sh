#!/bin/bash

# Blitwizard paths:
BLITWIZARD_BIN="REPLACE_BIN_PATH"
BLITWIZARD_TEMPLATES="REPLACE_TEMPLATES_PATH"
BLITWIZARD_BROWSER="REPLACE_BROWSER_PATH"

# Construct arguments:
ARGSTR=""
SKIPFIRST="yes"
for var in "$@"
do
    if [ "x$SKIPFIRST" = xyes ]; then
        SKIPFIRST="no"
    else
        ARGSTR="$ARGSTR \"$var\""
    fi
done

# Check script path being valid:
SCRIPTNAME="$1"
CHANGEDIR_OPTION=""
if [ -z "$SCRIPTNAME" ]; then
    SCRIPTNAME="$BLITWIZARD_BROWSER"
    CHANGEDIR_OPTION="-changedir "
fi
if [ ! -e "$SCRIPTNAME" ]; then
    echo "Error: the given path \"$SCRIPTNAME\" does not exist."
    exit 1;
fi

# Run Blitwizard, but make it find the templates:
$BLITWIZARD_BIN -templatepath "$BLITWIZARD_TEMPLATES" $CHANGEDIR_OPTION$SCRIPTNAME$ARGSTR


