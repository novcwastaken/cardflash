#!/bin/sh

# Exit if any of the command error & print all commands before execution
set -ex

if [ -d release ]; then
    rm -rf release
fi

mkdir -p release

cd release

cmake -DCMAKE_BUILD_TYPE=Release ..
