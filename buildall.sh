#!/bin/bash

if [ "$1" = "" ]; then
  echo "Usage: buildall.sh <prefix-directory>"
  exit 1
fi
PREFIX=$1

(cd libintense-io; ./cleanup.sh; ./bootstrap.sh)
rm -rf build-libintense-io
mkdir build-libintense-io
(cd build-libintense-io; \
 ../libintense-io/configure --prefix=$PREFIX; \
 make; make check; make install)

(cd libintense; ./cleanup.sh; ./bootstrap.sh)
rm -rf build-libintense
mkdir build-libintense
(cd build-libintense; \
 ../libintense/configure --prefix=$PREFIX; \
 make; make check; make install)

exit 0

(cd libintense-aep; ./cleanup.sh; ./bootstrap.sh)
rm -rf build-libintense-aep
mkdir build-libintense-aep
(cd build-libintense-aep; \
 ../libintense-aep/configure --prefix=$PREFIX; \
 make; make check; make install)
