#!/bin/sh

# Exit if any of the commands error & print all command before execution
set -ex

cmake --build build --target Cardflash -j$(nproc)
./build/Cardflash
