# ESM Potrace Wasm

A modern ESM build of the [Potrace](http://potrace.sourceforge.net/) library for use in the browser.

## Installation

```bash
npm install --save esm-potrace-wasm
```

## Usage

```js
import { loadFromCanvas } from 'esm-potrace-wasm';

// `canvas` is a reference to a canvas element.
// `config` is an optional configuration object.
try {
  const svg = await loadFromCanvas(canvas, config);
} catch (err) {
  console.error(err.name, err.message);
}
```

## Demo

ToDo.

## License

⚠️ GPL-3.0, due to the original [Potrace license](http://potrace.sourceforge.net/#license).

## Acknowledgements

Based on [@nturley](https://github.com/nturley)'s [fork](https://github.com/nturley/potrace-wasm) of
[@IguteChung](https://github.com/IguteChung)'s initial [potrace-wasm](https://github.com/IguteChung/potrace-wasm).
Unfortunately, the original author did not react on my Pull Request, so I forked.
