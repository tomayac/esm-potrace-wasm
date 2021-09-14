#!/usr/bin/env bash

set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR && rm -f lib/*

cd $DIR && emcc -O3 wasm/*.c --post-js src/potrace.js -s WASM=1 -s EXPORTED_FUNCTIONS='["_start"]' -s "ALLOW_MEMORY_GROWTH=1" -s "SINGLE_FILE=1" -o dist/index.js