#!/usr/bin/env bash

# Check if user provided a directory argument
if [ $# -ne 1 ]; 
then
    echo "Usage: $0 <directory>"
    exit 1
fi

directory=$1

# Check if the directory exists
if [ -d "$directory" ]; 
then
    echo "Compiling with cmake to directory '$directory'"

    cmake -S . -B ${directory}
    cmake --build ${directory}
else
    echo "Directory '$directory' does not exist."
    exit 1
fi