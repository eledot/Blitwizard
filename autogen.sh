#!/bin/sh
autoreconf -i -f || { echo "Autoconf generation failed! Do you have automake and autoconf installed? (libtool might also be needed)"; exit 1; }
