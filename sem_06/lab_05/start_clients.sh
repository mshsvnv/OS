#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <number of terminals> <server version>"
    exit 1
fi

konsole --layout cmd_json.json & KPID=$!

sleep 1

for ((i=1; i<=$1; i++))
do
    qdbus org.kde.konsole-$KPID /Sessions/$i runCommand "$2bakery_client localhost" & THISPID=$!
    if [ $i -eq 7 ]; then
        TO_KILL=$THISPID
    fi
done

# sleep 2

# kill $TO_KILL
# echo "Killing $TO_KILL"