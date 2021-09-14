#ifndef BACKEND_SVG_H
#define BACKEND_SVG_H

#include "potracelib.h"
#include "potrace.h"

struct svginfo_s
{
    int transform; /* 0 to disable <transform /> generation */
    int pathonly;  /* 1 to only return concated path data */
};
typedef struct svginfo_s svginfo_t;

int page_svg(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo, svginfo_t *svginfo);

#endif /* BACKEND_SVG_H */
