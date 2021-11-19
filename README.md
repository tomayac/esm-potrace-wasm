# ESM Potrace Wasm

A modern ESM build of the [Potrace](http://potrace.sourceforge.net/) library for use in the browser.

## Installation

```bash
npm install --save esm-potrace-wasm
```

## Usage

```js
import potrace from 'esm-potrace-wasm';

/**
 * The `imageBitmapSource` parameter is an `ImageBitmapSource`, that is any of:
 * - `HTMLImageElement`
 * - `SVGImageElement`
 * - `HTMLVideoElement`
 * - `HTMLCanvasElement`
 * - `ImageData`
 * - `ImageBitmap`
 * - `Blob`
 */
const svg = await potrace(
  imageBitmapSource,
  (options = {
    turdsize: 2,
    turnpolicy: 4,
    alphamax: 1,
    opticurve: 1,
    opttolerance: 0.2,
    pathonly: false,
  })
);
```

## Demo

This library is used in <a href="https://svgco.de/">SVGcode</a>.

<a href="https://svgco.de/">
  <img src="https://github.com/tomayac/SVGcode/raw/main/public/screenshots/desktop.png" alt="SVGcode application screenshot" width="707" height="497" />
</a>

## License

⚠️ GPL-3.0, due to the original [Potrace license](http://potrace.sourceforge.net/#license).

## Acknowledgements

Based on [@nturley](https://github.com/nturley)'s [fork](https://github.com/nturley/potrace-wasm) of
[@IguteChung](https://github.com/IguteChung)'s initial [potrace-wasm](https://github.com/IguteChung/potrace-wasm).
Unfortunately, the original author did not react on my
[Pull Request](https://github.com/IguteChung/potrace-wasm/pull/1), so I forked.
