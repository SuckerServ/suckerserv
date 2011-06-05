#!/bin/bash
PROJECT="$(tput bold ; tput setaf 3)SuckerServ$(tput sgr0)"
THREADS=`cat /proc/cpuinfo | grep processor | wc -l`
ARG_LENGTH=$#
if [[ $ARG_LENGTH > 2 ]]; then
  echo "Usage: $0 [--recompile] [--debug] - Build SuckerServ"
  echo "    --recompile    Delete \$COMPILEDIR before compiling SuckerServ again" 
  echo "    --debug        Make a debug build"
fi
STRCOMPILE="$(tput bold ; tput setaf 2)Compiling$(tput sgr0)"
COMPILEDIR="release_build"
COMPILEFLAGS=""
BUILDTYPE="$(tput bold ; tput setaf 6)release$(tput sgr0)"
if [ "$ARG_LENGTH" > 0 -a "$1" = "--debug" -o "$2" = "--debug" ]; then
  COMPILEDIR="debug-build"
  COMPILEFLAGS="-D CMAKE_BUILD_TYPE=DEBUG"
  BUILDTYPE="$(tput bold ; tput setaf 1)debug$(tput sgr0)"
fi
if [ "$ARG_LENGTH" > 0 -a "$1" = "--recompile" -o "$2" = "--recompile" ]; then
  STRCOMPILE="$(tput bold ; tput setaf 5)Recompiling$(tput sgr0)"
  rm -rf $COMPILEDIR
fi
if [ $THREADS = 1 ]; then
  echo "Unable to detect number of threads, using 1 thread."
  THREADS=1
fi
if [ ! -d $COMPILEDIR ]; then
  mkdir $COMPILEDIR
fi
cd $COMPILEDIR
STRTHREADS="threads"
if [ $THREADS = 1 ]; then
  STRTHREADS="thread"
fi
echo "$STRCOMPILE $PROJECT using $(tput bold ; tput setaf 4)$THREADS$(tput sgr0) $STRTHREADS ($BUILDTYPE build)"
time (cmake $COMPILEFLAGS .. ; make -j$THREADS install)
