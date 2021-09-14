#include <stdlib.h>
#include <string.h>

#include "potracelib.h"
#include "curve.h"
#include "decompose.h"
#include "trace.h"

/* default parameters */
static const potrace_param_t param_default = {
    2,                           /* turdsize */
    POTRACE_TURNPOLICY_MINORITY, /* turnpolicy */
    1.0,                         /* alphamax */
    1,                           /* opticurve */
    0.2,                         /* opttolerance */
};

/* Return a fresh copy of the set of default parameters, or NULL on
   failure with errno set. */
potrace_param_t *potrace_param_default(void)
{
  potrace_param_t *p;

  p = (potrace_param_t *)malloc(sizeof(potrace_param_t));
  if (!p)
  {
    return NULL;
  }
  memcpy(p, &param_default, sizeof(potrace_param_t));
  return p;
}

/* On success, returns a Potrace state st with st->status ==
   POTRACE_STATUS_OK. On failure, returns NULL if no Potrace state
   could be created (with errno set), or returns an incomplete Potrace
   state (with st->status == POTRACE_STATUS_INCOMPLETE, and with errno
   set). Complete or incomplete Potrace state can be freed with
   potrace_state_free(). */
potrace_state_t *potrace_trace(const potrace_param_t *param, const potrace_bitmap_t *bm)
{
  int r;
  path_t *plist = NULL;
  potrace_state_t *st;

  /* allocate state object */
  st = (potrace_state_t *)malloc(sizeof(potrace_state_t));
  if (!st)
  {
    return NULL;
  }

  /* process the image */
  r = bm_to_pathlist(bm, &plist, param);
  if (r)
  {
    free(st);
    return NULL;
  }

  st->status = POTRACE_STATUS_OK;
  st->plist = plist;
  st->priv = NULL; /* private state currently unused */

  /* partial success. */
  r = process_path(plist, param);
  if (r)
  {
    st->status = POTRACE_STATUS_INCOMPLETE;
  }

  return st;
}

/* free a Potrace state, without disturbing errno. */
void potrace_state_free(potrace_state_t *st)
{
  pathlist_free(st->plist);
  free(st);
}

/* free a parameter list, without disturbing errno. */
void potrace_param_free(potrace_param_t *p)
{
  free(p);
}
