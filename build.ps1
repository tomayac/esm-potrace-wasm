em++ -O3 -std=gnu++20 (ls wasm/*.c) --post-js src/potrace.js -s WASM=1 -s "ALLOW_MEMORY_GROWTH=1" -s "SINGLE_FILE=1" -s ASSERTIONS=1 -o dist/index.js

Add-Content -Path dist/index.js -Value "export { potrace, ready as init };"