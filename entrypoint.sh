#!/bin/bash
set -e

echo "Starting Node.js server..."
node html-interface/server.js &
NODE_PID=$!

echo "Waiting for Node.js server..."

# Wait until the server is accepting connections
until curl -s http://localhost:80/health > /dev/null; do
    sleep 1
done

echo "Node.js server ready. Starting transcoder..."

./bin/rr-transcoder &
C_PID=$!

# Wait for either process to exit
wait -n $NODE_PID $C_PID