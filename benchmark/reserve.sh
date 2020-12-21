#!/usr/bin/env bash

set -eu

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <node type> <timeout>"
    exit 1
fi

TYPE="$1"
TIMEOUT="$2"

# Make the reservation
RID=$(preserve -np 1 -t "$TIMEOUT" -native "-C $TYPE" | grep 'Reservation number' | cut -d' ' -f3 | cut -d':' -f1)

# Wait for the node to become active
while [ $(preserve -llist | awk -F'\t' "{if(\$1 == $RID){print \$7}}") != 'R' ]; do
    sleep 1
done

# Get the hostname
NODE=$(preserve -llist | awk -F'\t' "{if(\$1 == $RID){print \$9}}")
echo $NODE
