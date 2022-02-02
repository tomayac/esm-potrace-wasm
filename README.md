# ESM Potrace Wasm

A modern ESM build of the [Potrace](http://potrace.sourceforge.net/) library for use in the browser.

## Installation

```bash
npm install --save esm-potrace-wasm
```

## Usage

```js
import { potrace, init } from 'esm-potrace-wasm';

(async () => {
  // Initialize the module once.
  await init();

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
      extractcolors: true,
      posterizelevel: 2, // [1, 255]
      posterizationalgorithm: 0, // 0: simple, 1: interpolation
    })
  );
})();
```

## Developing

Modify `src/potrace/` and run `npm run build` to update the ESM build in `dist/`.
The emscripten build file is located at `./build.sh` (or `build.ps1` for Win32) respectively. A simple demo
that imports `dist/index.js` is available in `demo/` and can be started by running `npm start`.

## Apps using this

This library is used in <a href="https://svgco.de/">SVGcode</a>.

<a href="https://svgco.de/">
  <img src="https://github.com/tomayac/SVGcode/raw/main/public/screenshots/desktop.png" alt="SVGcode application screenshot" width="707" height="497" />
</a>

## License

⚠️ GPL-v2.0, due to the original [Potrace license](http://potrace.sourceforge.net/#license).

## Acknowledgements

Based on [@nturley](https://github.com/nturley)'s [fork](https://github.com/nturley/potrace-wasm) of
[@IguteChung](https://github.com/IguteChung)'s initial [potrace-wasm](https://github.com/IguteChung/potrace-wasm).
Unfortunately, the original author did not react on my
[Pull Request](https://github.com/IguteChung/potrace-wasm/pull/1), so I forked.
