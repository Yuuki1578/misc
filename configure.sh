#!/usr/bin/env sh

HOST_OS="$(uname -o)"
INCLUDE_DIR=""
LIB_DIR=""

case $HOST_OS in
  "Linux")
    INCLUDE_DIR="/usr/include"
    LIB_DIR="/usr/lib"
  ;;

  "Android")
    INCLUDE_DIR="/data/data/com.termux/files/usr/include"
    LIB_DIR="/data/data/com.termux/files/usr/lib"
  ;;

  *)
    echo "Host $HOST_OS is not supported"
    exit 1
  ;;
esac

case $1 in
  "--install")
    test -d build || mkdir build
    cmake -S . -B build
    cmake --build build

    cp -r include/libmisc $INCLUDE_DIR/libmisc
    cp build/libmisc.a $LIB_DIR/libmisc.a
    echo "Done, now i live in your filesystem"
  ;;

  "--uninstall")
    rm -rf $INCLUDE_DIR/libmisc
    rm $LIB_DIR/libmisc.a
    echo "Good bye!"
  ;;

  *)
    echo "Usage: ./configure.sh --install | --uninstall"
    exit 1
  ;;
esac
