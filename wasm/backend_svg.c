/* The SVG backend of Potrace. */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "potracelib.h"
#include "curve.h"
#include "potrace.h"
#include "backend_svg.h"
#include "lists.h"
#include "auxiliary.h"
#include "hexutils.h"

/* ---------------------------------------------------------------------- */
/* path-drawing auxiliary functions */

/* coordinate quantization */
static inline point_t unit(dpoint_t p)
{
  point_t q;

  q.x = (long)(floor(p.x * UNIT + .5));
  q.y = (long)(floor(p.y * UNIT + .5));
  return q;
}

static point_t cur;
static char lastop = 0;
static int column = 0;
static int newline = 1;

static void shiptoken(FILE *fout, const char *token)
{
  int c = strlen(token);
  if (!newline && column + c + 1 > 75)
  {
    fprintf(fout, " ");
    column = 0;
    newline = 1;
  }
  else if (!newline)
  {
    fprintf(fout, " ");
    column++;
  }
  fprintf(fout, "%s", token);
  column += c;
  newline = 0;
}

static void ship(FILE *fout, const char *fmt, ...)
{
  va_list args;
  static char buf[4096]; /* static string limit is okay here because
			    we only use constant format strings - for
			    the same reason, it is okay to use
			    vsprintf instead of vsnprintf below. */
  char *p, *q;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  buf[4095] = 0;
  va_end(args);

  p = buf;
  while ((q = strchr(p, ' ')) != NULL)
  {
    *q = 0;
    shiptoken(fout, p);
    p = q + 1;
  }
  shiptoken(fout, p);
}

static void svg_moveto(FILE *fout, dpoint_t p, transform_t *t)
{
  cur = unit(p);
  long x = cur.x, y = cur.y;
  if (t)
  {
    float tx = x * t->scalex + t->origx, ty = y * t->scaley + t->origy;
    ship(fout, "M%.1f %.1f", tx, ty);
  }
  else
  {
    ship(fout, "M%ld %ld", x, y);
  }

  lastop = 'M';
}

static void svg_rmoveto(FILE *fout, dpoint_t p, transform_t *t)
{
  point_t q;

  q = unit(p);
  long x = q.x - cur.x, y = q.y - cur.y;
  if (t)
  {
    float tx = x * t->scalex, ty = y * t->scaley;
    ship(fout, "m%.1f %.1f", tx, ty);
  }
  else
  {
    ship(fout, "m%ld %ld", x, y);
  }

  cur = q;
  lastop = 'm';
}

static void svg_lineto(FILE *fout, dpoint_t p, transform_t *t)
{
  point_t q;

  q = unit(p);
  long x = q.x - cur.x, y = q.y - cur.y;
  if (t)
  {
    float tx = x * t->scalex, ty = y * t->scaley;
    char *str = "l%.1f %.1f";
    if (lastop == 'l')
    {
      str++;
    }
    ship(fout, str, tx, ty);
  }
  else
  {
    char *str = "l%ld %ld";
    if (lastop == 'l')
    {
      str++;
    }
    ship(fout, str, x, y);
  }

  cur = q;
  lastop = 'l';
}

static void svg_curveto(FILE *fout, dpoint_t p1, dpoint_t p2, dpoint_t p3, transform_t *t)
{
  point_t q1, q2, q3;

  q1 = unit(p1);
  q2 = unit(p2);
  q3 = unit(p3);
  long x1 = q1.x - cur.x, y1 = q1.y - cur.y,
       x2 = q2.x - cur.x, y2 = q2.y - cur.y,
       x3 = q3.x - cur.x, y3 = q3.y - cur.y;
  if (t)
  {
    float tx1 = x1 * t->scalex,
          ty1 = y1 * t->scaley,
          tx2 = x2 * t->scalex,
          ty2 = y2 * t->scaley,
          tx3 = x3 * t->scalex,
          ty3 = y3 * t->scaley;
    char *str = "c%.1f %.1f %.1f %.1f %.1f %.1f";
    if (lastop == 'c')
    {
      str++;
    }
    ship(fout, str, tx1, ty1, tx2, ty2, tx3, ty3);
  }
  else
  {
    char *str = "c%ld %ld %ld %ld %ld %ld";
    if (lastop == 'c')
    {
      str++;
    }
    ship(fout, str, x1, y1, x2, y2, x3, y3);
  }
  cur = q3;
  lastop = 'c';
}

/* ---------------------------------------------------------------------- */
/* functions for converting a path to an SVG path element */

/* Explicit encoding. If abs is set, move to first coordinate
   absolutely. */
static int svg_path(FILE *fout, potrace_curve_t *curve, int abs, transform_t *transform)
{
  int i;
  dpoint_t *c;
  int m = curve->n;

  c = curve->c[m - 1];
  if (abs)
  {
    svg_moveto(fout, c[2], transform);
  }
  else
  {
    svg_rmoveto(fout, c[2], transform);
  }

  for (i = 0; i < m; i++)
  {
    c = curve->c[i];
    switch (curve->tag[i])
    {
    case POTRACE_CORNER:
      svg_lineto(fout, c[1], transform);
      svg_lineto(fout, c[2], transform);
      break;
    case POTRACE_CURVETO:
      svg_curveto(fout, c[0], c[1], c[2], transform);
      break;
    }
  }
  newline = 1;
  shiptoken(fout, "z");
  return 0;
}

/* ---------------------------------------------------------------------- */
/* Backend. */

/* public interface for SVG */

svgpath_group_transform_t init_group_transform_data(imginfo_t *imginfo)
{
  svgpath_group_transform_t tr;
  
  tr.bbox.bboxx = imginfo->trans.bb[0] + imginfo->lmar + imginfo->rmar;
  tr.bbox.bboxy = imginfo->trans.bb[1] + imginfo->tmar + imginfo->bmar;
  tr.transform.origx = imginfo->trans.orig[0] + imginfo->lmar;
  tr.transform.origy = tr.bbox.bboxy - imginfo->trans.orig[1] - imginfo->bmar;
  tr.transform.scalex = imginfo->trans.scalex / UNIT;
  tr.transform.scaley = -imginfo->trans.scaley / UNIT;
  return tr;
}

void write_paths_transparent_rec(FILE *fout, potrace_path_t *tree, transform_t *transform, int pathonly)
{
  potrace_path_t *p, *q;

  for (p = tree; p; p = p->sibling)
  {
    if (!pathonly)
    {
      column = fprintf(fout, "<path d=\"");
    }
    newline = 1;
    lastop = 0;

    svg_path(fout, &p->curve, 1, transform);

    for (q = p->childlist; q; q = q->sibling)
    {
      svg_path(fout, &q->curve, 0, transform);
    }

    if (!pathonly)
    {
      fprintf(fout, "\"/>");
    }
    else
    {
      fprintf(fout, " ");
    }

    for (q = p->childlist; q; q = q->sibling)
    {
      write_paths_transparent_rec(fout, q->childlist, transform, pathonly);
    }
  }
}

void add_svg_header(FILE* fout, double bboxx, double bboxy)
{
  /* header */
  fprintf(fout, "<?xml version=\"1.0\" standalone=\"no\"?>");
  fprintf(fout, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\"");
  fprintf(fout, " \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">");

  /* set bounding box and namespace */
  fprintf(fout, "<svg version=\"1.0\" xmlns=\"http://www.w3.org/2000/svg\"");
  fprintf(fout, " width=\"%f\" height=\"%f\" viewBox=\"0 0 %f %f\"", bboxx, bboxy, bboxx, bboxy);
  fprintf(fout, " preserveAspectRatio=\"xMidYMid meet\">");
}

void add_footer(FILE* fout)
{
  fprintf(fout, "</svg>");
}

void start_group(FILE *fout, transform_t *transform, uint8_t* hex_color)
{
  fprintf(fout, "<g transform=\"");
  if (transform->origx != 0 || transform->origy != 0)
  {
    fprintf(fout, "translate(%f,%f) ", transform->origx, transform->origy);
  }
  fprintf(fout, "scale(%f,%f)\" ", transform->scalex, transform->scaley);
  fprintf(fout, "fill=\"#%s\" stroke=\"none\">", hex_color);
}

void end_group(FILE* fout)
{
  fprintf(fout, "</g>");
}

int page_svg(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo, svginfo_t *svginfo)
{
  transform_t *transform = NULL;
  auto tr = init_group_transform_data(imginfo);
  
  if (!svginfo->transform)
  {
    transform_t t = tr.transform;
    transform = &t;
  }

  if (!svginfo->pathonly)
  {
    add_svg_header(fout, tr.bbox.bboxx, tr.bbox.bboxy);
    /* use a "group" tag to establish coordinate system and style */
    if (svginfo->transform)
    {
      char* c = "000000";
      start_group(fout, &(tr.transform), (uint8_t*)c);
    }
  }

  write_paths_transparent_rec(fout, plist, transform, svginfo->pathonly);

  if (!svginfo->pathonly)
  {
    /* write footer */
    if (svginfo->transform)
    {
      end_group(fout);
    }
    add_footer(fout);
  }
  fflush(fout);

  return 0;
}
