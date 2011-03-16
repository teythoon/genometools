/*
  Copyright (c) 2010 Dirk Willrodt <willrodt@zbh.uni-hamburg.de>
  Copyright (c) 2010 Center for Bioinformatics, University of Hamburg

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

#include <stdio.h>

#include "core/array2dim_api.h"
#include "core/encseq_api.h"
#include "core/format64.h"
#include "core/log_api.h"
#include "core/logger.h"
#include "core/ma.h"
#include "core/mathsupport.h"

#include "match/eis-voiditf.h"
#include "match/esa-seqread.h"
#include "match/esa-shulen.h"
#include "match/idx-limdfs.h"
#include "match/sarr-def.h"
#include "match/shu-dfs.h"
#include "match/shu-divergence.h"
#include "match/shu-encseq-gc.h"
#include "match/shu_unitfile.h"

#include "match/shu-genomediff.h"

int gt_genomediff_shu(GtLogger *logger,
                      const GtGenomediffArguments *arguments,
                      GtTimer *timer,
                      GtError *err)
{
  int had_err = 0;
  double **div = NULL,
         *gc_contents = NULL;
  unsigned long num_of_genomes = 0,
                *genome_length = NULL,
                i_idx, j_idx;
  uint64_t **shulen = NULL;
  GtStrArray *genome_names = NULL;
  const GtStrArray *file_names = NULL;
  const GtEncseq *encseq = NULL;
  Genericindex *genericindexSubject = NULL;
  Sequentialsuffixarrayreader *ssar = NULL;

  if (arguments->with_esa)
  {
    gt_error_check(err);
    if (timer != NULL)
    {
      gt_timer_show_progress(timer, "load sequential sa reader", stdout);
    }
    ssar =
      gt_newSequentialsuffixarrayreaderfromfile(gt_str_get(
                                                  arguments->indexname),
                                                SARR_LCPTAB |
                                                SARR_SUFTAB |
                                                SARR_ESQTAB |
                                                SARR_SSPTAB,
                                                arguments->scan
                                                ? SEQ_scan : SEQ_mappedboth,
                                                err);
    if (ssar == NULL)
    {
      had_err = -1;
    }
    if (!had_err)
    {
      encseq = gt_encseqSequentialsuffixarrayreader(ssar);
      num_of_genomes = gt_encseq_num_of_files(encseq);
      gt_array2dim_calloc(shulen, num_of_genomes, num_of_genomes);
      genome_length = gt_calloc((size_t) num_of_genomes,
                                sizeof (unsigned long));
      if (timer != NULL)
      {
        gt_timer_show_progress(timer, "dfs esa index", stdout);
      }
      if (gt_get_multiesashulengthdist(ssar,
                                       encseq,
                                       shulen,
                                       logger,
                                       err) != 0)
      {
        had_err = -1;
      }
    }
  }
  else
  {
    unsigned long numofchars = 0,
                  totallength = 0;
    const GtAlphabet *alphabet;
    const FMindex *subjectindex = NULL;

    if (timer != NULL)
    {
      gt_timer_show_progress(timer, "map generic index", stdout);
    }
    genericindexSubject = genericindex_new(gt_str_get(arguments->indexname),
                                           arguments->with_esa,
                                           true,
                                           false,
                                           true,
                                           arguments->user_max_depth,
                                           logger,
                                           err);
    if (genericindexSubject == NULL)
    {
      gt_assert(gt_error_is_set(err));
      had_err = -1;
    }
    else
    {
      encseq = genericindex_getencseq(genericindexSubject);
      /* TODO add function to fill num_of_genomes and genome_names */
      genome_names = gt_str_array_new();
      if (arguments->with_units)
      {
        had_err = gt_read_genomediff_unitfile(arguments->unitfile,
                                    encseq,
                                    genome_names,
                                    &num_of_genomes,
                                    logger,
                                    err);
        if (!had_err)
        {
          fprintf(stderr, "successfully loaded unitfile\n");
        }
      }
      num_of_genomes = gt_encseq_num_of_files(encseq);
      gt_array2dim_calloc(shulen,
                          num_of_genomes,
                          num_of_genomes);
      genome_length = gt_calloc((size_t) num_of_genomes,
                                sizeof (unsigned long));
    }

    if (!had_err)
    {
      alphabet = gt_encseq_alphabet(encseq);
      if (!gt_alphabet_is_dna(alphabet) &&
          (!arguments->traverse_only ||
           !arguments->shulen_only))
      {
        gt_error_set(err,"error: sequences need to be dna to calculate gc!");
        had_err = -1;
      }
      else
      {
        numofchars = (unsigned long) gt_alphabet_num_of_chars(alphabet);
        totallength = genericindex_get_totallength(genericindexSubject);
        subjectindex = genericindex_get_packedindex(genericindexSubject);
      }
    }

    if (!had_err)
    {
      gt_assert(shulen);
      gt_assert(subjectindex);
      had_err = gt_pck_calculate_shulen(subjectindex,
                                        encseq,
                                        shulen,
                                        numofchars,
                                        totallength,
                                        !arguments->traverse_only,
                                        timer,
                                        logger,
                                        err);
    }
    if (!had_err && arguments->traverse_only)
    {
      gt_logger_log(logger, "stopping after traversal");
      genericindex_delete(genericindexSubject);
      gt_free(genome_length);
      if (shulen != NULL)
      {
        gt_array2dim_delete(shulen);
      }
      gt_assert(timer != NULL);
      return had_err;
    }
  }

  if (!had_err)
  {
    gt_assert(genome_length);
    /* TODO check if not NULL (if set bevor during reading of unitfile) */
    file_names = gt_encseq_filenames(encseq);
    for (i_idx = 0UL; i_idx < num_of_genomes && !had_err; i_idx++)
    {
      genome_length[i_idx] =
        (unsigned long) gt_encseq_effective_filelength(encseq, i_idx) - 1;
    }
  }
  if (!had_err)
  {
    gt_assert(shulen);
    if (timer != NULL)
    {
      gt_timer_show_progress(timer, "calculate avg", stdout);
    }
    if (arguments->shulen_only)
    {
      printf("# sum of shulen\n%lu\n", num_of_genomes);
    }
    gt_array2dim_calloc(div, num_of_genomes, num_of_genomes);
    gt_assert(genome_length);
    for (i_idx = 0; i_idx < num_of_genomes; i_idx++)
    {
      unsigned long length_i;
      length_i = genome_length[i_idx];
      if (arguments->shulen_only)
      {
        printf("%s\t", gt_str_array_get(file_names, i_idx));
      }
      for (j_idx = 0; j_idx < num_of_genomes; j_idx++)
      {
        if (j_idx == i_idx)
        {
          if (arguments->shulen_only)
          {
            printf("0\t");
          }
        }
        else
        {
          if (arguments->shulen_only)
          {
            printf(Formatuint64_t"\t", arguments->with_esa ?
                              PRINTuint64_tcast(shulen[j_idx][i_idx]) :
                              PRINTuint64_tcast(shulen[i_idx][j_idx]));
          }
          else
          {
            div[i_idx][j_idx] = arguments->with_esa ?
                                  ((double) shulen[j_idx][i_idx]) / length_i :
                                  ((double) shulen[i_idx][j_idx]) / length_i;
          }
        }
      }
      if (arguments->shulen_only)
      {
        printf("\n");
      }
    }
  }
  if (!arguments->shulen_only && !had_err)
  {
    gt_assert(div);
    if (timer != NULL)
    {
      gt_timer_show_progress(timer, "calculate gc", stdout);
    }
    gc_contents = gt_encseq_get_gc(encseq,
                                  false,
                                  err);
    if (gc_contents == NULL)
    {
      had_err = -1;
    }
    gt_logger_log(logger, "table of avg shulens");
    if (!had_err && gt_logger_enabled(logger))
    {
      for (i_idx = 0; i_idx < num_of_genomes; i_idx++)
      {
        printf("# %s\t", gt_str_array_get(file_names, i_idx));
        for (j_idx = 0; j_idx < num_of_genomes; j_idx++)
        {
          if (i_idx == j_idx)
          {
            printf("0\t\t");
          }
          else
          {
            printf("%f\t", div[i_idx][j_idx]);
          }
        }
        printf("\n");
      }
    }
    if (!had_err)
    {
      double *ln_n_fac;

      gt_assert(div);
      gt_assert(genome_length);
      gt_assert(gc_contents);

      if (timer != NULL)
      {
        gt_timer_show_progress(timer, "precalculate ln_n_fac", stdout);
      }
      ln_n_fac = gt_get_ln_n_fac(arguments->max_ln_n_fac);
      if (timer != NULL)
      {
        gt_timer_show_progress(timer, "calculate divergence", stdout);
      }
      for (i_idx = 0; i_idx < num_of_genomes; i_idx++)
      {
        for (j_idx = i_idx+1; j_idx < num_of_genomes; j_idx++)
        {
          double query_gc, query_shulen;
          unsigned long subject_len;
          if (gt_double_smaller_double(div[i_idx][j_idx],
                                       div[j_idx][i_idx]))
          { /* S=j_idx Q=i_idx */
            query_gc = gc_contents[i_idx];
            query_shulen = div[i_idx][j_idx];
            subject_len = genome_length[j_idx];
          }
          else
          {
            if (gt_double_smaller_double(div[j_idx][i_idx],
                                         div[i_idx][j_idx]))
            { /* S=i_idx Q=j_idx */
              query_gc = gc_contents[j_idx];
              query_shulen = div[j_idx][i_idx];
              subject_len = genome_length[i_idx];
            }
            else
            {
              if (gt_double_smaller_double(fabs(gc_contents[i_idx]-0.5),
                                           fabs(gc_contents[j_idx]-0.5)))
              { /* S=i_idx Q=j_idx XXX check this if right*/
                query_gc = gc_contents[j_idx];
                query_shulen = div[j_idx][i_idx];
                subject_len = genome_length[i_idx];
              }
              else
              { /* S=j_idx Q=i_idx */
                query_gc = gc_contents[i_idx];
                query_shulen = div[i_idx][j_idx];
                subject_len = genome_length[j_idx];
              }
            }
          }

          div[i_idx][j_idx] =
            gt_divergence(arguments->divergence_rel_err,
                          arguments->divergence_abs_err,
                          arguments->divergence_m,
                          arguments->divergence_threshold,
                          query_shulen,
                          subject_len,
                          query_gc,
                          ln_n_fac,
                          arguments->max_ln_n_fac);
          div[j_idx][i_idx] = div[i_idx][j_idx];
        }
      }
      gt_free(ln_n_fac);
    }
    gt_logger_log(logger, "table of divergences");
    if (!had_err && gt_logger_enabled(logger))
    {
      gt_assert(div);
      for (i_idx = 0; i_idx < num_of_genomes; i_idx++)
      {
        printf("# %s\t", gt_str_array_get(file_names, i_idx));
        for (j_idx = 0; j_idx < num_of_genomes; j_idx++)
        {
          if (i_idx == j_idx)
          {
            printf("0\t\t");
            continue;
          }
          printf("%f\t", div[i_idx][j_idx]);
        }
        printf("\n");
      }
    }
    if (!had_err)
    {
      gt_assert(div);
      if (timer != NULL)
      {
        gt_timer_show_progress(timer, "calculate kr", stdout);
      }
      printf("# Table of Kr\n%lu\n", num_of_genomes);
      for (i_idx = 0; i_idx < num_of_genomes; i_idx++)
      {
        printf("%s\t", gt_str_array_get(file_names, i_idx));
        for (j_idx = 0; j_idx < num_of_genomes; j_idx++)
        {
          if ( i_idx == j_idx )
          {
            printf("0\t\t");
          }
          else
          {
            printf("%f\t", gt_calculateKr(div[i_idx][j_idx]));
          }
        }
        printf("\n");
      }
    }
  }
  if (arguments->with_esa)
  {
    if (ssar != NULL)
    {
      gt_freeSequentialsuffixarrayreader(&ssar);
    }
  }
  else
  {
    genericindex_delete(genericindexSubject);
  }

  gt_free(genome_length);
  gt_free(gc_contents);
  if (shulen != NULL)
  {
    gt_array2dim_delete(shulen);
  }
  if (div != NULL)
  {
    gt_array2dim_delete(div);
  }
  return had_err;
}
