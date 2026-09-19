#!/bin/sh

cmake --build build --target Cardflash -j$(nproc)
