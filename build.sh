#!/usr/bin/env bash

set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR && rm -f lib/*

cd $DIR && em++ -O3 -std=gnu++20 wasm/*.c --post-js src/potrace.js -s WASM=1 -s EXPORTED_FUNCTIONS='["_start"]' -s "ALLOW_MEMORY_GROWTH=1" -s "SINGLE_FILE=1" -s "MODULARIZE=1" -s "EXPORT_ES6=1" -o dist/temp.js