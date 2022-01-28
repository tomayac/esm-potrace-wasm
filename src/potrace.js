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
    },
    options
  );
  const colorFilter = (r, g, b, a) =>
    a && 0.2126 * r + 0.7152 * g + 0.0722 * b < 128;
  /** @type {ImageData} */
  let imageData;
  const constructorName = imageBitmapSource.constructor.name;
  if (constructorName === 'Blob') {
    imageData = await (async () => {
      return new Promise((resolve) => {
        const url = URL.createObjectURL(imageBitmapSource);
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
    const canvasImageSource = imageBitmapSource;
    canvas.width = Number(canvasImageSource.width);
    canvas.height = Number(canvasImageSource.height);
    ctx.drawImage(canvasImageSource, 0, 0, canvas.width, canvas.height);
    imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
    document.body.append(canvas);
  } else {
    imageData = imageBitmapSource;
  }
  const start = wrapStart();
  await ready();
  const result = start(
    imageData.data,
    imageData.width,
    imageData.height,
    true,
    options.pathonly,
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
