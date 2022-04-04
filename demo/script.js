import potraceModule from '../dist/index.js';

(async () => {
  const Module = await potraceModule();
  // @connorjclark Unsure about the next two lines.
  await Module.ready;
  const potrace = Module._start;

  const pre1 = document.querySelector('#pre1');
  const pre2 = document.querySelector('#pre2');
  const pre3 = document.querySelector('#pre3');
  const pre4 = document.querySelector('#pre4');
  const div1 = document.querySelector('#div1');
  const div2 = document.querySelector('#div2');
  const div3 = document.querySelector('#div3');
  const div4 = document.querySelector('#div4');

  const blob1 = await fetch('./logo.png').then((response) => response.blob());
  const blob2 = await fetch('./avatar.jpg').then((response) => response.blob());

  potrace(blob1, {
    turdsize: 2,
    turnpolicy: 4,
    alphamax: 1,
    opticurve: 1,
    opttolerance: 0.2,
    pathonly: false,
    extractcolors: true,
    posterizelevel: 2,
    posterizationalgorithm: 0, // 0: simple, 1: interpolation
  }) .then((svg) => {
    =div1.innerHTML = svg;
    svg = svg.replaceAll('><', '>\n<');
    pre1.textContent = svg + '\n\n(' + svg.length + ')';
  });

  potrace(blob1, {
    turdsize: 2,
    turnpolicy: 4,
    alphamax: 1,
    opticurve: 1,
    opttolerance: 0.2,
    pathonly: false,
    extractcolors: false,
  }).then((svg) => {
    div2.innerHTML = svg;
    svg = svg.replaceAll('><', '>\n<');
    pre2.textContent = svg + '\n\n(' + svg.length + ')';
  });

  potrace(blob2, {
    turdsize: 2,
    turnpolicy: 4,
    alphamax: 1,
    opticurve: 1,
    opttolerance: 0.2,
    pathonly: false,
    extractcolors: true,
    posterizelevel: 5,
    posterizationalgorithm: 1, // 0: simple, 1: interpolation
  }).then((svg) => {
    div3.innerHTML = svg;
    svg = svg.replaceAll('><', '>\n<');
    pre3.textContent = svg + '\n\n(' + svg.length + ')';
  });

  potrace(blob2, {
    turdsize: 2,
    turnpolicy: 4,
    alphamax: 1,
    opticurve: 1,
    opttolerance: 0.2,
    pathonly: false,
    extractcolors: false,
  }).then((svg) => {
    div4.innerHTML = svg;
    svg = svg.replaceAll('><', '>\n<');
    pre4.textContent = svg + '\n\n(' + svg.length + ')';
  });
})();
