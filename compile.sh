#!/bin/bash

# Check if the g++ compiler is installed
if ! command -v g++ &> /dev/null; then
    echo "Error: g++ compiler not found. Please install g++ before running this script."
    exit 1
fi

# Check if a C++ source file name is provided as an argument
if [ -z "$1" ]; then
    echo "Error: Please provide the C++ source file name as an argument."
    echo "Usage: $0 <source_file.cpp>"
    exit 1
fi

# Check if the provided file exists
if [ ! -f "$1" ]; then
    echo "Error: File '$1' not found."
    exit 1
fi

# Extract the file name without extension
filename=$(basename "$1" .cpp)

# Compile the C++ code with pthread library and -O3 optimization flag
g++ -std=c++20 -pthread -O3 "$1" -o "$filename"

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable '$filename' created."
else
    echo "Compilation failed. Please check for errors in your C++ code."
fi
