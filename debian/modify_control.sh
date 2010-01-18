#!/bin/sh

if [ "$1" = "add" ]
then
    echo "Adding conboy-midgard from debian/control"
    grep -q "conboy-midgard" debian/control
    if [ $? -eq 1 ]
    then
        cat debian/control_midgard >> debian/control
    fi
    exit
fi


if [ "$1" = "remove" ]
then
    echo "Removing conboy-midgard from debian/control"
    sed -i '/conboy-midgard/,$d' debian/control
    exit
fi


echo "ERROR: Please specify 'add' or 'remove' as parameter."
exit
