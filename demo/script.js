import potrace from '../dist/index.min.js';

(async () => {
  const pre = document.querySelector('pre');
  const div = document.querySelector('div');

  const blob = await fetch('./snail.png').then((response) => response.blob());
  let svg = await potrace(blob, {
    turdsize: 1,
    turnpolicy: 4,
    alphamax: 1,
    opticurve: 1,
    opttolerance: 0.2,
    pathonly: false,
  });
  div.innerHTML = svg;
  svg = svg.replaceAll('><', '>\n<');
  pre.textContent = svg + '\n\n(' + svg.length + ')';
})();
