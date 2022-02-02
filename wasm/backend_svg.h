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

struct svgbouding_box_s
{
  double bboxx;
  double bboxy;
};
typedef struct svgbouding_box_s svgbouding_box_t;

struct svgpath_group_transform_s
{
  transform_t transform;
  svgbouding_box_t bbox;
};
typedef struct svgpath_group_transform_s svgpath_group_transform_t;

svgpath_group_transform_t init_group_transform_data(imginfo_t *imginfo);

void add_svg_header(FILE* fout, double bboxx, double bboxy);
void add_footer(FILE* fout);
void start_group(FILE *fout, transform_t *transform, uint8_t* hex_color);
void end_group(FILE* fout);

void write_paths_transparent_rec(FILE *fout, potrace_path_t *tree, transform_t *transform, int pathonly);

int page_svg(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo, svginfo_t *svginfo);

#endif /* BACKEND_SVG_H */
