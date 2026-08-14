export interface PotraceOptions {
  turdsize?: number;
  turnpolicy?: number;
  alphamax?: number;
  opticurve?: number;
  opttolerance?: number;
  pathonly?: boolean;
  extractcolors?: boolean;
  posterizelevel?: number;
  posterizationalgorithm?: number;
}

export function potrace(
  imageBitmapSource: ImageBitmapSource,
  options: PotraceOptions & { pathonly: true }
): Promise<string[]>;
export function potrace(
  imageBitmapSource: ImageBitmapSource,
  options?: PotraceOptions & { pathonly?: false }
): Promise<string>;
export function potrace(
  imageBitmapSource: ImageBitmapSource,
  options?: PotraceOptions
): Promise<string | string[]>;
export function init(): Promise<void>;
