/*
  Copyright (c) 2006-2008 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2006-2008 Center for Bioinformatics, University of Hamburg

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

#ifndef SCORE_FUNCTION_H
#define SCORE_FUNCTION_H

#include "core/score_matrix.h"

typedef struct GT_ScoreFunction GT_ScoreFunction;

GT_ScoreFunction* gt_score_function_new(GT_ScoreMatrix*, /* takes ownership  */
                                 int deletion_score, int insertion_score);
int            gt_score_function_get_score(const GT_ScoreFunction*,
                                       unsigned int, unsigned int);
const int**    gt_score_function_get_scores(const GT_ScoreFunction*);
int            gt_score_function_get_deletion_score(const GT_ScoreFunction*);
int            gt_score_function_get_insertion_score(const GT_ScoreFunction*);
void           gt_score_function_delete(GT_ScoreFunction*);

#endif
