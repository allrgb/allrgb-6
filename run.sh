#!/bin/bash

set -e

declare -a examples=("allrgb_v0" "julia_set" "allrgb_v1" "allrgb_v2" "allrgb_v3")

echo "Compiling:"
for e in "${examples[@]}"
do
   echo " * $e"
   gcc "$e".c -o "$e"
   rm "$e".ppm || true
done

echo "Running:"
for e in "${examples[@]}"
do
   echo " * $e"
   ./"$e" >> "$e".ppm
done