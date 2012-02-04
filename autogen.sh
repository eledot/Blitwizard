#!/bin/sh
autoreconf -i -f || { echo "Autoconf generation failed! Do you have autotools and autoconf installed?"; exit 1; }
