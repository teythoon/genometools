/*
  Copyright (c) 2010-2011 Gordon Gremme <gordon@gremme.org>

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

#ifndef ID_TO_MD5_STREAM_API_H
#define ID_TO_MD5_STREAM_API_H

#include <stdio.h>
#include "extended/node_stream_api.h"
#include "extended/region_mapping_api.h"

/* Implements the <GtNodeStream> interface. A <GtIDToMD5Stream> converts
   ``regular'' sequence IDs to MD5 fingerprints. */
typedef struct GtIDToMD5Stream GtIDToMD5Stream;

/* Create a <GtIDToMD5Stream> object which converts ``regular'' sequence IDs
   from nodes it retrieves from its <in_stream> to MD5 fingerprints (with the
   help of the given <region_mapping>). If <substitute_target_ids> is true, the
   IDs of Target attributes are also converted to MD5 fingerprints.
   Takes ownership of <region_mapping>! */
GtNodeStream* gt_id_to_md5_stream_new(GtNodeStream *in_stream,
                                      GtRegionMapping *region_mapping,
                                      bool substitute_target_ids);

#endif
