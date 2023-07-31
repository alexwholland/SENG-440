#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <source_file>"
    exit 1
fi

source_file="$1"
executable="program"

# Check if the source file exists
if [ ! -f "$source_file" ]; then
    echo "Error: File '$source_file' not found."
    exit 1
fi

# Compile the source file using gcc with debug symbols
gcc -o "$executable" -g "$source_file"

# Check if the compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Running perf stat..."
    # Run the program with perf stat
    perf stat "./$executable"
else
    echo "Compilation failed."
fi
