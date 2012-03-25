#!/bin/bash

value="`cmakeargh --help 2> /dev/null | grep 'cmake version'`"

if [ -z "$value" ]; then
    exit 0;
else
    exit 1;
fi

