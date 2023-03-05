#!/bin/bash
# Author: shaswathts

FILE=$1
STR=$2

# Check if ["$filesdir"] is specified & represent a directory on the filesystem
if [ -n "$FILE" ]
then
    # Creating directory and file specified in the first argument
    mkdir -p "$(dirname $FILE)" && touch "$FILE"
    
    # Check if the [$searchstr] argument is specified
    if [ -n "$STR" ]
    then
        echo "Writing string $STR to file $FILE"
        echo $STR > $FILE;

    else
        exit $?

    fi

else 
    exit $?
fi