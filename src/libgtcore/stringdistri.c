/*
  Copyright (c) 2007-2008 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2007-2008 Center for Bioinformatics, University of Hamburg

  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <assert.h>
#include "libgtcore/cstr.h"
#include "libgtcore/hashtable.h"
#include "libgtcore/ma.h"
#include "libgtcore/stringdistri.h"
#include "libgtcore/unused.h"

struct StringDistri {
  Hashtable *hashdist;
  unsigned long num_of_occurrences;
};

StringDistri* stringdistri_new(void)
{
  StringDistri *sd;
  sd = ma_malloc(sizeof *sd);
  sd->hashdist = hashtable_new(HASH_STRING, ma_free_func, ma_free_func);
  sd->num_of_occurrences = 0;
  return sd;
}

void stringdistri_add(StringDistri *d, const char *key)
{
  unsigned long *valueptr;
  assert(d && key);
  valueptr = hashtable_get(d->hashdist, (void*) key);
  if (!valueptr) {
    valueptr = ma_malloc(sizeof *valueptr);
    *valueptr = 1;
    hashtable_add(d->hashdist, cstr_dup(key), valueptr);
  }
  else
    (*valueptr)++;
  d->num_of_occurrences++;
}

typedef struct {
  StringDistriIterFunc func;
  void *data;
  unsigned long num_of_occurrences;
} ForeachInfo;

static int foreach_iterfunc(void *key, void *value, void *data,
                            UNUSED Error *err)
{
  unsigned long occurrences;
  ForeachInfo *info;
  error_check(err);
  assert(key && value && data);
  occurrences = *(unsigned long*) value;
  info = (ForeachInfo*) data;
  info->func(key, occurrences, (double) occurrences / info->num_of_occurrences,
             info->data);
  return 0;
}

void stringdistri_foreach(const StringDistri *d, StringDistriIterFunc func,
                          void *data)
{
  ForeachInfo info;
  int rval;
  assert(d);
  if (d->hashdist) {
    info.func = func;
    info.data = data;
    info.num_of_occurrences = d->num_of_occurrences;
    rval = hashtable_foreach_ao(d->hashdist, foreach_iterfunc, &info, NULL);
    assert(!rval); /* foreach_iterfunc() is sane */
  }
}

void stringdistri_delete(StringDistri *d)
{
  if (!d) return;
  hashtable_delete(d->hashdist);
  ma_free(d);
}
