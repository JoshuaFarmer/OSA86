#!/bin/bash

# Check if a directory was provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <directory>"
    exit 1
fi

directory="$1"

# Verify the directory exists
if [ ! -d "$directory" ]; then
    echo "Error: Directory '$directory' does not exist."
    exit 1
fi

# Count lines in specified file types
echo "Counting lines in .c, .asm, .s, and .h files in '$directory' and subdirectories..."

total_lines=0

# Process each file type
for ext in c asm s h; do
    while IFS= read -r -d '' file; do
        lines=$(wc -l < "$file")
        printf "%6d %s\n" "$lines" "$file"
        total_lines=$((total_lines + lines))
    done < <(find "$directory" -type f -name "*.$ext" -print0)
done

echo "-----------------------------"
echo "Total lines: $total_lines"