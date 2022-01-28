import { potrace, init } from '../dist/index.js';

(async () => {
  const pre1 = document.querySelector('#pre1');
  const pre2 = document.querySelector('#pre2');
  const div1 = document.querySelector('#div1');
  const div2 = document.querySelector('#div2');

  const blob = await fetch('./snail.png').then((response) => response.blob());

  await init();

  const start = Date.now();
  await potrace(blob, {
    turdsize: 1,
    turnpolicy: 4,
    alphamax: 1,
    opticurve: 1,
    opttolerance: 0.2,
    pathonly: false,
  }).then((svg) => {
    div1.innerHTML = svg;
    svg = svg.replaceAll('><', '>\n<');
    pre1.textContent = svg + '\n\n(' + svg.length + ')';
  });
console.log(Date.now() - start);
  potrace(blob, {
    turdsize: 2,
    turnpolicy: 4,
    alphamax: 1,
    opticurve: 1,
    opttolerance: 0.2,
    pathonly: false,
  }).then((svg) => {
    div2.innerHTML = svg;
    svg = svg.replaceAll('><', '>\n<');
    pre2.textContent = svg + '\n\n(' + svg.length + ')';
  });
})();
