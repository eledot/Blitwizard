#!/bin/bash

# This tests whether cmake is present. Returns 0 if yes, otherwise 1

value="`cmake --help 2> /dev/null | grep 'cmake version'`"

if [ -z "$value" ]; then
    exit 1;
else
    exit 0;
fi

