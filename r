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
SRC=".tmp/main.cpp"
OUT=".tmp/outfile"
for CODEFILE in "${INFILES[@]}"; do
  echo "Testing '$CODEFILE'..."
  EXE=".tmp/main"
  cp "$CODEFILE" "$SRC"
  g++ $CXXFLAGS "$SRC" -o "$EXE" --debug || exit 1
  INFILE=$(echo $CODEFILE | sed 's/cpp/in/')
  OUTFILE=$(echo $CODEFILE | sed 's/cpp/out/')
  if [ ! -e "$INFILE" ]; then INFILE="/dev/null"; fi
  if [ -n "$VALGRIND" ]; then EXE="valgrind $EXE"; fi
  time $EXE < "$INFILE" > "$OUT"
  diff "$OUT" "$OUTFILE" || exit 1
done
