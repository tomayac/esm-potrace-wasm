#ifndef DECOMPOSE_H
#define DECOMPOSE_H

#include "potracelib.h"
#include "curve.h"

int bm_to_pathlist(const potrace_bitmap_t *bm, path_t **plistp, const potrace_param_t *param);

#endif /* DECOMPOSE_H */
