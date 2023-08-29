#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "potrace.h"
#include "potracelib.h"
#include "bitmap.h"
#include "backend_svg.h"
#include "hexutils.h"

#undef abs

#include <map>
#include <vector>
#include <algorithm>
#include <memory>

std::vector<char> frame;
char* svgStr = nullptr;

static uint8_t get_quantized_value(uint8_t color, std::vector<float>& levels)
{
    if (color == 255 || color == 0)
        return color;
    float c = (float)color / 255.0f;
    auto l = *(std::lower_bound(std::begin(levels), std::end(levels), c));
    auto r = l + levels[1];
    auto val = std::abs(l - c) > std::abs(r - c) ? r : l;

    return 255.0f * val;
}

static uint8_t get_quantized_value_simple(uint8_t color, uint8_t level)
{
    float q_c = (color / level) * level + level/2;
    return std::clamp(q_c, 0.0f, 255.0f);
}

static bool color_filter(float r, float g, float b, float a)
{
    return a > 0 && (0.2126 * r + 0.7152 * g + 0.0722 * b < 128);
}

static std::vector<float> get_ranges(uint8_t levels)
{
    std::vector<float> table_values(levels + 1);
    float base = 1.0f / levels;
    for (uint8_t i = 0; i <= levels; ++i)
    {
        table_values[i] = base * i;
    }
    return std::move(table_values);
}

/* ---------------------------------------------------------------------- */
/* calculations with bitmap dimensions, positioning etc */

/* determine the dimensions of the output based on command line and
   image dimensions, and optionally, based on the actual image outline. */
static void calc_dimensions(imginfo_t *imginfo)
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


const char *start_monochromatic(
    uint32_t* image,
    imginfo_t* imginfo,
    potrace_param_t* param,
    svginfo_t* svginfo
    )
{
    int width = imginfo->pixwidth;
    int height = imginfo->pixheight;
    potrace_bitmap_t *bm = bm_new(width, height);
    for (int i = 0; i < width * height; i++)
    {
        // each uint8_t contains 8 pixels from rightmost bit.
        int x = i % width;
        int y = height - (i / width) - 1;
        uint8_t* color = (uint8_t*)&(image[i]);
        BM_UPUT(bm, x, y, color_filter(color[0], color[1], color[2], color[3] ? 1 : 0));
    }

    potrace_state_t *st = potrace_trace(param, bm);
    if (!st || st->status != POTRACE_STATUS_OK)
    {
        fprintf(stderr, "trace error: %s\n", strerror(errno));
        exit(2);
    }

    bm_free(bm);

    // start convert to svg.
    char *buf;
    size_t len;
    FILE *stream = open_memstream(&buf, &len);
    int r = page_svg(stream, st->plist, imginfo, svginfo);
    if (r)
    {
        fprintf(stderr, "page_svg error: %s\n", strerror(errno));
        exit(2);
    }

    // release resource.
    fclose(stream);
    potrace_state_free(st);

    return buf;
}

const char *start_color(
    uint32_t* image,
    imginfo_t* imginfo,
    potrace_param_t* param,
    svginfo_t* svginfo,
    uint8_t quantlevel,
    uint8_t posterization_algorithm
    )
{
    int width = imginfo->pixwidth;
    int height = imginfo->pixheight;
    
    auto tr = init_group_transform_data(imginfo);
    transform_t* trans = nullptr;
    if (!svginfo->transform)
    {
        transform_t t = tr.transform;
        trans = &t;
    }

    int index = 0;
    std::map<uint32_t, std::vector<std::pair<uint32_t, point_t>>> colors;
    

    std::function<uint8_t(uint8_t)> map_value;
    int invlevel = ceil(256.0f / (quantlevel + 1));
    auto levels = get_ranges(quantlevel);
    if (posterization_algorithm == 0)
    {
        map_value = [&](uint8_t c) { return get_quantized_value_simple(c, invlevel); };
    }
    else
    {
        map_value = [&](uint8_t c) { return get_quantized_value(c, levels); };
    }

    for (int i = 0; i < width * height; i++)
    {
        int x = i % width;
        int y = height - (i / width) - 1;
        uint32_t pixel = image[i];
        uint8_t* color = (uint8_t*)&pixel;
        color[0] = map_value(color[0]);
        color[1] = map_value(color[1]);
        color[2] = map_value(color[2]);
        if (color[3] > 0)
        {
            color[3] = 255;
            point_t p;
            p.x = x;
            p.y = y;
            auto it = colors.find(pixel);
            if (it == std::end(colors))
            {
                std::vector<std::pair<uint32_t, point_t>> points;
                points.reserve(500);
                colors[pixel] = points;
            }
            colors[pixel].push_back({i, p});
        }
    }

    char *buf;
    size_t len;
    FILE *fout = open_memstream(&buf, &len);

    if (!svginfo->pathonly)
    {
        add_svg_header(fout, tr.bbox.bboxx, tr.bbox.bboxy);
    }

    for (auto& color : colors)
    {
        potrace_bitmap_t *bm = bm_new(width, height);
        for (auto& c : color.second)
        {
            BM_UPUT(bm, c.second.x, c.second.y, 1);
        }
        potrace_state_t *st = potrace_trace(param, bm);
        if (!st || st->status != POTRACE_STATUS_OK)
        {
            fprintf(stderr, "trace error: %s\n", strerror(errno));
            exit(2);
        }

        if (!st->plist)
        {
            continue;
        }

        if (svginfo->transform)
        {
            uint8_t* c = (uint8_t*)&(color.first);
            auto hex_color = rgb_to_hex(c[0], c[1], c[2]);
            start_group(fout, &(tr.transform), hex_color);
            free(hex_color);
        }
        // conver the trace to image information for svg backend generation.
        bm_free(bm);    
        write_paths_transparent_rec(fout, st->plist, trans, svginfo->pathonly);
        if (svginfo->transform)
        {
            end_group(fout);
        }
        potrace_state_free(st);
    }
    if (!svginfo->pathonly)
    {
        add_footer(fout);
    }
    fflush(fout);

    // release resource.
    fclose(fout);
    
    return buf;
}

const char *start(
    int width,
    int height,
    uint8_t transform,
    uint8_t pathonly,
    uint8_t extract_colors,
    uint8_t quantlevel,
    uint8_t posterization_algorithm,
    int turdsize,
    int turnpolicy,
    double alphamax,
    int opticurve,
    double opttolerace
    )
{
    if (frame.size() != width * height * 4)
    {
        return "";
    }
    uint32_t* image = (uint32_t*)frame.data();

    imginfo_t imginfo = {
        .pixwidth = width,
        .pixheight = height,
    };
    
    potrace_param_t param = {
        .turdsize = turdsize,
        .turnpolicy = turnpolicy,
        .alphamax = alphamax,
        .opticurve = opticurve,
        .opttolerance = opttolerace,
    };

    calc_dimensions(&imginfo);

    svginfo_t svginfo = {
        .transform = transform,
        .pathonly = pathonly,
    };

    if (!extract_colors) 
    {
        svgStr = (char*)start_monochromatic(image, &imginfo, &param, &svginfo);
    }
    else 
    {
        svgStr = (char*)start_color(image, &imginfo, &param, &svginfo, quantlevel, posterization_algorithm);
    }
    return svgStr;
}

unsigned long allocate_frame(
      int width,
      int height
    )
{
    frame.resize(width * height * 4);
    return reinterpret_cast<unsigned long>(frame.data());
}

void free_resources()
{
    if (svgStr != nullptr)
    {
        free(svgStr);
        svgStr = nullptr;
    }
    frame.clear();
}