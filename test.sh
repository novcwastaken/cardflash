#!/bin/sh

# Exit if any command errors & print all commands before execution
set -ex

cmake --build build --target tests -j$(nproc)
build/tests