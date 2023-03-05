#!/bin/sh
# Author: shaswathts

filesdir=$1
searchstr=$2

# Check if ["$filesdir"] is specified & represent a directory on the filesystem
if [ -n "$filesdir" ] && [ -d $filesdir ]
then
    # Check if the [$searchstr] argument is specified
    if [ -n "$searchstr" ]
    then
        grep -Rioh $searchstr $filesdir

    else
        echo "Exit with return value $?, Please specify the searchstr"
        exit $?
    fi

else 
    echo "$filesdir is not a dir"
    echo "Exit with return value $?"
    exit $?
fi