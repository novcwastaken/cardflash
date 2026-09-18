#!/bin/sh

cmake --build build -j$(nproc)
./build/Cardflash
