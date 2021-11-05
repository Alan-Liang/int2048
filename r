#!/bin/bash

case "$1" in
"all" | "-a" | "a")
  INFILES=(data/**/**.cpp)
  ;;

"1" | "2")
  INFILES=(data/*$1/*.cpp)
  if [ -n "$2" ]; then
    INFILES=(data/*$1/$2.cpp)
  fi
  ;;

*)
  echo "Usage: ./r all|a|-a|1|2"
  exit 1
esac

if [ "${#INFILES[@]}" = "0" ]; then
  echo "Input not found."
  exit 0
fi

if echo ${INFILES[*]} | grep '\*' > /dev/null; then
  echo "Input not found."
  exit 0
fi

mkdir -p .tmp
cp int2048.hpp .tmp/
for CODEFILE in "${INFILES[@]}"; do
  echo "Testing '$CODEFILE'..."
  cp "$CODEFILE" .tmp/main.cpp
  g++ $CXXFLAGS ".tmp/main.cpp" -o .tmp/main --debug || exit 1
  INFILE=$(echo $CODEFILE | sed 's/cpp/in/')
  OUTFILE=$(echo $CODEFILE | sed 's/cpp/out/')
  if [ ! -e "$INFILE" ]; then INFILE="/dev/null"; fi
  time .tmp/main < "$INFILE" > .tmp/outfile
  diff .tmp/outfile "$OUTFILE" || exit 1
done
