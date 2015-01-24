#!/bin/sh

NAME=freakz

if [ "$(grep "AC_INIT(${NAME}" configure.ac)" = "" ]; then
  echo "autogen.sh must be run in PWD of ${NAME} source"
  exit 1
fi
grep "^[0-9]" ChangeLog | awk '{split($0,a,"\t"); printf("%s, %s, %s\n", a[1] ,a[2], a[3]);}' | sort -u > AUTHORS
