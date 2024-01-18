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
    # Compile
    echo "Compiling with cmake to directory '/$directory/'..."

    cmake -S . -B ${directory}
    cmake --build ${directory}

    # Execute
    echo "Starting server executable in '/$directory/'..."

    ./${directory}/server

else
    echo "Directory '/$directory/' does not exist."
    exit 1
fi