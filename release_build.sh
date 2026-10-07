#!/bin/sh

cmake --build release --target Cardflash -j$(nproc)
