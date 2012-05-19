#!/bin/bash

# Please run this script with current working directory being "scripts/"
# - otherwise it won't find the required files.

grep AC_INIT ../configure.ac | sed -e "s/AC_INIT[(][[]blitwizard[]], [[]//g" | sed -e "s/[]])//g"
