/* eslint-disable no-unused-vars */
/**
 * This file will be inserted to generated output when building the library.
 */

/**
 * @param colorFilter return true if given pixel will be traced.
 * @param transform whether add the <transform /> tag to reduce generated svg length.
 * @param pathonly only returns concatenated path data.
 */
const defaultConfig = {
  colorFilter: (r, g, b, a) => a && 0.2126 * r + 0.7152 * g + 0.0722 * b < 128,
  transform: true,
  pathonly: false,
};

/**
 * @param config for customizing.
 * @returns merged config with default value.
 */
function buildConfig(config) {
  if (!config) {
    return Object.assign({}, defaultConfig);
  }
  const merged = Object.assign({}, config);
  for (const prop in defaultConfig) {
    if (!config.hasOwnProperty(prop)) {
      merged[prop] = defaultConfig[prop];
    }
  }
  return merged;
}

/**
 * @returns promise to wait for wasm loaded.
 */
function ready() {
  return new Promise((resolve) => {
    if (runtimeInitialized) {
      resolve();
      return;
    }
    Module.onRuntimeInitialized = () => {
      resolve();
    };
  });
}

/**
 * @param canvas to be converted for svg.
 * @param config for customizing.
 * @returns promise that emits a svg string or path data array.
 */
async function loadFromCanvas(canvas, config, params) {
  const ctx = canvas.getContext('2d');
  const imagedata = ctx.getImageData(0, 0, canvas.width, canvas.height).data;
  return loadFromImageData(
    imagedata,
    canvas.width,
    canvas.height,
    config,
    params
  );
}

/**
 * @param imagedata to be converted for svg.
 * @param width for the imageData.
 * @param height for the imageData.
 * @param config for customizing.
 * @returns promise that emits a svg string or path data array.
 */
async function loadFromImageData(
  imagedata,
  width,
  height,
  config,
  params = {}
) {
  const start = wrapStart();
  const data = new Array(Math.ceil(imagedata.length / 32)).fill(0);
  const c = buildConfig(config);

  for (let i = 0; i < imagedata.length; i += 4) {
    const r = imagedata[i];
    const g = imagedata[i + 1];
    const b = imagedata[i + 2];
    const a = imagedata[i + 3];

    if (c.colorFilter(r, g, b, a)) {
      // each number contains 8 pixels from rightmost bit.
      const index = Math.floor(i / 4);
      data[Math.floor(index / 8)] += 1 << index % 8;
    }
  }

  await ready();
  const result = start(
    data,
    width,
    height,
    c.transform,
    c.pathonly,
    (params.turdsize = params.turdsize !== undefined ? params.turdsize : 0.2),
    (params.turnpolicy =
      params.turnpolicy !== undefined ? params.turnpolicy : 4),
    (params.alphamax = params.alphamax !== undefined ? params.alphamax : 1),
    (params.opticurve = params.opticurve !== undefined ? params.opticurve : 1),
    (params.opttolerance =
      params.opttolerance !== undefined ? params.opttolerance : 0.2)
  );

  if (config.pathonly) {
    return result
      .split('M')
      .filter((path) => path)
      .map((path) => 'M' + path);
  }
  return result;
}

/**
 * @returns wrapped function for start.
 */
function wrapStart() {
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
  module.exports = { loadFromCanvas, loadFromImageData };
}
