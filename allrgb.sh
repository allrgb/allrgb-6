#!/bin/bash

set -e

if [ $# -eq 0 ]; then
    echo "No arguments provided"
    task_build="BUILD"
    task_run="RUN"
fi

while test $# -gt 0
do
    case "$1" in
        --build) task_build="BUILD"
            ;;
        --run) task_run="RUN"
            ;;
    esac
    shift
done

declare -a examples=("allrgb_v0" "julia_set" "allrgb_v1" "allrgb_v2" "allrgb_v3")


if [ "$task_build" == "BUILD" ]; then
    echo "Compiling:"
    for e in "${examples[@]}"
    do
       echo " * $e"
       gcc "$e".c -o "$e"
       rm "$e".ppm || true
    done
fi


if [ "$task_run" == "RUN" ]; then
    echo "Running:"
    for e in "${examples[@]}"
    do
       echo " * $e"
       ./"$e" >> "$e".ppm
    done
fi

exit 0
