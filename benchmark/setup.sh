#!/usr/bin/env bash

set -eu

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <server node> <client nodes...>"
    exit 1
fi

SERVER="$1"
shift
CLIENTS="$@"

export ECHIDNA_BENCHMARK=$(realpath $(dirname $0))
export ECHIDNA_ROOT=$(realpath "$ECHIDNA_BENCHMARK/..")

mkdir -p log

echo "Starting server"
# Start the server
ssh "$SERVER" "$ECHIDNA_ROOT/build/echidna-server" 2> log/server.log &

# Give the server some time to start
sleep 2
echo "Starting clients"

# Start the clients
for NODE in "$CLIENTS"; do
    ssh "$NODE" "$ECHIDNA_ROOT/build/echidna-client $SERVER" 2> "log/client-$NODE.log" &
done
