#!/bin/sh
autoreconf -i -f || { echo "Autoconf generation failed! Do you have autotools installed?"; exit 1; }
