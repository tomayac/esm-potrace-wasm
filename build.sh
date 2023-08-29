#!/usr/bin/env bash

set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd $DIR && rm -f lib/*

cd $DIR && em++ -O3 -std=gnu++20 wasm/*.c --post-js src/potrace.js -s WASM=1 -s EXPORTED_FUNCTIONS='["_start", "_free_resources", "_allocate_frame"]' -s EXTRA_EXPORTED_RUNTIME_METHODS=["cwrap"] -s "ALLOW_MEMORY_GROWTH=1" -s "SINGLE_FILE=1" -o dist/temp.js