#ifndef BACKEND_SVG_H
#define BACKEND_SVG_H

#include <stdint.h>

#include "potracelib.h"
#include "potrace.h"

struct svginfo_s
{
    int transform; /* 0 to disable <transform /> generation */
    int pathonly;  /* 1 to only return concated path data */
};
typedef struct svginfo_s svginfo_t;

struct transform_s
{
  double origx;
  double origy;
  double scalex;
  double scaley;
};
typedef struct transform_s transform_t;

void write_paths_transparent_rec(FILE *fout, potrace_path_t *tree, transform_t *transform, int pathonly, uint8_t pixels[], int width, uint32_t color);

int page_svg(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo, svginfo_t *svginfo, uint8_t pixels[], int width, uint32_t color);

#endif /* BACKEND_SVG_H */
