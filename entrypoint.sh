#!/bin/bash
set -e

echo "Starting Node.js server..."
node html-interface/server.js &
SERVER_PID=$!

echo "Starting Node.js bridge..."
node bridge/bridge.js &
OTHER_SERVER_PID=$!

echo "Waiting for Node.js server..."

# Wait until the server is accepting connections
until curl -s http://localhost:80/health > /dev/null; do
    sleep 1
done

echo "Node.js server ready. Starting transcoder..."

./bin/rr-transcoder &
TRANSCODER_PID=$!

# Wait for any process to exit
wait -n $SERVER_PID $OTHER_SERVER_PID $TRANSCODER_PID
