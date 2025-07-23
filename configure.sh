# The Fuck Around and Find Out License v0.1
# Copyright (C) 2025 Awang Destu Pradhana
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "software"), to deal
# in the software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the software, and to permit persons to whom the software is
# furnished to do so, subject to the following conditions:
#
# 1. The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the software.
#
# 2. The software shall be used for Good, not Evil. The original author of the
# software retains the sole and exclusive right to determine which uses are
# Good and which uses are Evil.
#
# 3. The software is provided "as is", without warranty of any kind, express or
# implied, including but not limited to the warranties of merchantability,
# fitness for a particular purpose and noninfringement. In no event shall the
# authors or copyright holders be liable for any claim, damages or other
# liability, whether in an action of contract, tort or otherwise, arising from,
# out of or in connection with the software or the use or other dealings in the
# software.

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
