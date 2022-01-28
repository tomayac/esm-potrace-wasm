#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "potrace.h"
#include "potracelib.h"
#include "bitmap.h"
#include "backend_svg.h"
#include <map>
#include <vector>

/* ---------------------------------------------------------------------- */
/* calculations with bitmap dimensions, positioning etc */

/* determine the dimensions of the output based on command line and
   image dimensions, and optionally, based on the actual image outline. */
static void calc_dimensions(imginfo_t *imginfo, potrace_path_t *plist)
{
    /* we take care of a special case: if one of the image dimensions is
     0, we change it to 1. Such an image is empty anyway, so there
     will be 0 paths in it. Changing the dimensions avoids division by
     0 error in calculating scaling factors, bounding boxes and
     such. This doesn't quite do the right thing in all cases, but it
     is better than causing overflow errors or "nan" output in
     backends.  Human users don't tend to process images of size 0
     anyway; they might occur in some pipelines. */
    if (imginfo->pixwidth == 0)
    {
        imginfo->pixwidth = 1;
    }
    if (imginfo->pixheight == 0)
    {
        imginfo->pixheight = 1;
    }

    /* apply default dimension to width, height, margins */
    imginfo->lmar = 0;
    imginfo->rmar = 0;
    imginfo->tmar = 0;
    imginfo->bmar = 0;

    /* start with a standard rectangle */
    trans_from_rect(&imginfo->trans, imginfo->pixwidth, imginfo->pixheight);

    /* if width and height are still variable, tenatively use the
     default scaling factor of 72dpi (for dimension-based backends) or
     1 (for pixel-based backends). For fixed-size backends, this will
     be adjusted later to fit the page. */
    imginfo->width = imginfo->trans.bb[0];
    imginfo->height = imginfo->trans.bb[1];

    /* apply scaling */
    trans_scale_to_size(&imginfo->trans, imginfo->width, imginfo->height);
}

static bool colorFilter(float r, float g, float b, float a) {
    return a > 1 && 0.2126 * r + 0.7152 * g + 0.0722 * b < 200;
}


const char *start(
    uint8_t pixels[],
    int width,
    int height,
    uint8_t transform,
    uint8_t pathonly,
    int turdsize,
    int turnpolicy,
    double alphamax,
    int opticurve,
    double opttolerace
    )
{
    // initialize the bitmap with given pixels.
    //potrace_bitmap_t *bm = bm_new(width, height);

    imginfo_t imginfo = {
        .pixwidth = width,
        .pixheight = height,
    };

    uint32_t* image = (uint32_t*)pixels;

    int index = 0;
    std::map<uint32_t, std::vector<std::pair<uint32_t, point_t>>> colors;
    for (int i = 0; i < width * height; i++)
    {
        int x = i % width;
        int y = height - (i / width) - 1;
        uint32_t pixel = image[i];
        uint8_t* color = (uint8_t*)&pixel;
        uint8_t r = color[0];
        uint8_t g = color[1];
        uint8_t b = color[2];
        uint8_t a = color[3];
        if (colorFilter(r, g, b, a) ? 1 : 0)
        {
            point_t p;
            p.x = x;
            p.y = y;
            auto it = colors.find(pixel);
            if (it == std::end(colors))
            {
                std::vector<std::pair<uint32_t, point_t>> points;
                points.reserve(500);
                points.push_back({i, p});
                colors[pixel] = points;
            }
            else
            {
                colors[pixel].push_back({i, p});
            }
        }
        //BM_UPUT(bm, x, y, colorFilter(r, g, b, a) ? 1 : 0);
    }

    transform_t *tr = NULL;
    double bboxx = imginfo.trans.bb[0] + imginfo.lmar + imginfo.rmar;
    double bboxy = imginfo.trans.bb[1] + imginfo.tmar + imginfo.bmar;
    double origx = imginfo.trans.orig[0] + imginfo.lmar;
    double origy = bboxy - imginfo.trans.orig[1] - imginfo.bmar;
    double scalex = imginfo.trans.scalex / UNIT;
    double scaley = -imginfo.trans.scaley / UNIT;

    char *buf;
    size_t len;
    FILE *fout = open_memstream(&buf, &len);
    svginfo_t svginfo = {
        .transform = transform,
        .pathonly = pathonly,
    };

    if (!svginfo.pathonly)
    {
        /* header */
        fprintf(fout, "<?xml version=\"1.0\" standalone=\"no\"?>");
        fprintf(fout, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\"");
        fprintf(fout, " \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">");

        /* set bounding box and namespace */
        fprintf(fout, "<svg version=\"1.0\" xmlns=\"http://www.w3.org/2000/svg\"");
        fprintf(fout, " width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\"",
            width, height, width, height);
        fprintf(fout, " preserveAspectRatio=\"xMidYMid meet\">");

        /* use a "group" tag to establish coordinate system and style */
        if (svginfo.transform)
        {
            fprintf(fout, "<g transform=\"");
           
            fprintf(fout, "translate(%f,%d) ", 0.0, height);
            fprintf(fout, "scale(%f,%f)\" ", 0.1, -0.1);
            fprintf(fout, "fill=\"#000000\" stroke=\"none\">");
        }
    }
    if (!svginfo.transform)
    {
        transform_t t = {
            origx = origx,
            origy = origy,
            scalex = scalex,
            scaley = scaley,
        };
        tr = &t;
    }

    for (auto& color : colors)
    {
        potrace_bitmap_t *bm = bm_new(width, height);
        for (auto& c : color.second)
        {
            BM_UPUT(bm, c.second.x, c.second.y, 1);
        }
        // start the potrace algorithm.
        potrace_param_t param = {
            .turdsize = turdsize,
            .turnpolicy = turnpolicy,
            .alphamax = alphamax,
            .opticurve = opticurve,
            .opttolerance = opttolerace,
        };

        potrace_state_t *st = potrace_trace(&param, bm);
        if (!st || st->status != POTRACE_STATUS_OK)
        {
            fprintf(stderr, "trace error: %s\n", strerror(errno));
            exit(2);
        }

        // conver the trace to image information for svg backend generation.
        bm_free(bm);

        // calculte the dimension of image.
        calc_dimensions(&imginfo, st->plist);

        // start convert to svg.
        // char *buf;
        // size_t len;
        // FILE *stream = open_memstream(&buf, &len);
        // svginfo_t svginfo = {
       //     .transform = transform,
        //     .pathonly = pathonly,
        // };
        // int r = page_svg(stream, st->plist, &imginfo, &svginfo, pixels, width, color->first);
        // if (r)
        // {
        //     fprintf(stderr, "page_svg error: %s\n", strerror(errno));
        //     exit(2);
        // }

        write_paths_transparent_rec(fout, st->plist, tr, svginfo.pathonly, pixels, width, color.first);
        potrace_state_free(st);


    }
    if (!svginfo.pathonly)
    {
        /* write footer */
        if (svginfo.transform)
        {
            fprintf(fout, "</g>");
        }
        fprintf(fout, "</svg>");
    }
    fflush(fout);

    // release resource.
    fclose(fout);
    
    return buf;
}