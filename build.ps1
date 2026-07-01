if (Test-Path $dist/index.js) {
    Remove-Item dist/index.js
}

em++ -O3 -std=c++20 (ls wasm/*.c) --post-js src/potrace.js -s WASM=1 -s 'EXPORTED_FUNCTIONS=["_start","_malloc","_free"]' -s 'EXPORTED_RUNTIME_METHODS=["cwrap","HEAPU8"]' -s "ALLOW_MEMORY_GROWTH=1" -s "SINGLE_FILE=1" -o dist/temp.js