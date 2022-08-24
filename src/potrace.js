/**
 * A modern ESM build of the Potrace (http://potrace.sourceforge.net/) library for use in the browser.
 * Copyright (C) 2021 Thomas Steiner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/** @license SPDX-License-Identifier: GPL-2.0-or-later */

/**
 * @param {ImageBitmapSource} imageBitmapSource
 * @param {Object} options
 * @return {Promise<string>}
 */
const potrace = async (imageBitmapSource, options = {}) => {
  options = Object.assign(
    {
      turdsize: 2,
      turnpolicy: 4,
      alphamax: 1,
      opticurve: 1,
      opttolerance: 0.2,
      pathonly: false,
      extractcolors: true,
      posterizelevel: 5,
      posterizationalgorithm: 1,
    },
    options
  );
  /** @type {ImageData} */
  let imageData;
  const constructorName = imageBitmapSource.constructor.name;
  if (constructorName === 'Blob') {
    imageData = await (async () => {
      return new Promise((resolve) => {
        const url = URL.createObjectURL(
          /** @type {Blob} */ (imageBitmapSource)
        );
        const image = new Image();
        image.onload = () => {
          const canvas = document.createElement('canvas');
          const ctx = canvas.getContext('2d');
          if (!ctx) {
            throw new Error('Canvas is not supported.');
          }
          const canvasImageSource = /** @type {CanvasImageSource} */ (image);
          canvas.width = Number(canvasImageSource.width);
          canvas.height = Number(canvasImageSource.height);
          ctx.drawImage(canvasImageSource, 0, 0, canvas.width, canvas.height);
          resolve(ctx.getImageData(0, 0, canvas.width, canvas.height));
        };
        image.src = url;
      });
    })();
  } else if (
    constructorName === 'HTMLImageElement' ||
    constructorName === 'SVGImageElement' ||
    constructorName === 'HTMLVideoElement' ||
    constructorName === 'HTMLCanvasElement' ||
    constructorName === 'ImageBitmap'
  ) {
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    if (!ctx) {
      throw new Error('Canvas is not supported.');
    }
    const canvasImageSource = /** @type {CanvasImageSource} */ (
      imageBitmapSource
    );
    canvas.width = Number(canvasImageSource.width);
    canvas.height = Number(canvasImageSource.height);
    ctx.drawImage(canvasImageSource, 0, 0, canvas.width, canvas.height);
    imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
  } else {
    imageData = /** @type {ImageData} */ (imageBitmapSource);
  }
  const start = wrapStart();
  await ready();
  const result = start(
    imageData.data,
    imageData.width,
    imageData.height,
    true,
    options.pathonly,
    options.extractcolors,
    options.posterizelevel,
    options.posterizationalgorithm,
    options.turdsize,
    options.turnpolicy,
    options.alphamax,
    options.opticurve,
    options.opttolerance
  );
  if (options.pathonly) {
    return result
      .split('M')
      .filter((path) => path)
      .map((path) => 'M' + path);
  }
  return result;
};

/**
 * @return {Promise<void>} promise to wait for wasm loaded.
 */
function ready() {
  return new Promise((resolve) => {
    // @ts-ignore
    if (runtimeInitialized) {
      resolve();
      return;
    }
    // @ts-ignore
    Module.onRuntimeInitialized = () => {
      resolve();
    };
  });
}

/**
 * @returns wrapped function for start.
 */
function wrapStart() {
  // @ts-ignore
  return cwrap('start', 'string', [
    'array', // pixels
    'number', // width
    'number', // height
    'number', // transform
    'number', // pathonly
    'number', // extractColors
    'number', // quantlevel
    'number', // posterizationAlgorithm
    'number', // turdsize
    'number', // turnpolicy
    'number', // alphamax
    'number', // opticurve
    'number', // opttolerance
  ]);
}

// export the functions in server env.
if (typeof module !== 'undefined') {
  const init = ready;
  module.exports = { potrace, init };
}
