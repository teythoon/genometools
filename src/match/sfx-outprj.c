/*
  Copyright (c) 2007 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
  Copyright (c) 2007 Center for Bioinformatics, University of Hamburg

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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include "core/endianess_api.h"
#include "core/fa.h"
#include "core/filelengthvalues.h"
#include "core/str_array.h"
#include "core/str.h"
#include "core/unused_api.h"
#include "seqpos-def.h"
#include "defined-types.h"
#include "format64.h"
#include "spacedef.h"
#include "esa-fileend.h"
#include "readmode-def.h"
#include "encodedsequence.h"
#include "stamp.h"

#define PRJSPECIALOUT(VAL)\
        fprintf(outprj,"%s=" FormatSeqpos "\n",#VAL,\
                PRINTSeqposcast(getencseq##VAL(encseq)))

static void showprjinfo(FILE *outprj,
                        Readmode readmode,
                        const GtEncodedsequence *encseq,
                        unsigned int prefixlength,
                        GT_UNUSED const Definedunsignedint *maxdepth,
                        Seqpos numoflargelcpvalues,
                        Seqpos maxbranchdepth,
                        const DefinedSeqpos *longest)
{
  Seqpos totallength;
  unsigned long numofsequences;

  totallength = gt_encodedsequence_total_length(encseq);
  fprintf(outprj,"totallength=" FormatSeqpos "\n",PRINTSeqposcast(totallength));
  PRJSPECIALOUT(specialcharacters);
  PRJSPECIALOUT(specialranges);
  PRJSPECIALOUT(realspecialranges);
  PRJSPECIALOUT(lengthofspecialprefix);
  PRJSPECIALOUT(lengthofspecialsuffix);
  numofsequences = gt_encodedsequence_num_of_sequences(encseq);
  fprintf(outprj,"numofsequences=%lu\n",numofsequences);
  fprintf(outprj,"numofdbsequences=%lu\n",numofsequences);
  fprintf(outprj,"numofquerysequences=0\n");
  if (longest->defined)
  {
    fprintf(outprj,"longest=" FormatSeqpos "\n",
            PRINTSeqposcast(longest->valueseqpos));
  }
  fprintf(outprj,"prefixlength=%u\n",prefixlength);
  /*
  if (maxdepth->defined)
  {
    fprintf(outprj,"maxdepth=%u\n",maxdepth->valueunsignedint);
  }
  */
  fprintf(outprj,"largelcpvalues=" FormatSeqpos "\n",
                   PRINTSeqposcast(numoflargelcpvalues));
  fprintf(outprj,"maxbranchdepth=" FormatSeqpos "\n",
                   PRINTSeqposcast(maxbranchdepth));
  fprintf(outprj,"integersize=%u\n",
                  (unsigned int) (sizeof (Seqpos) * CHAR_BIT));
  fprintf(outprj,"littleendian=%c\n",gt_is_little_endian() ? '1' : '0');
  fprintf(outprj,"readmode=%u\n",(unsigned int) readmode);
}

int outprjfile(const GtStr *indexname,
               Readmode readmode,
               const GtEncodedsequence *encseq,
               unsigned int prefixlength,
               const Definedunsignedint *maxdepth,
               Seqpos numoflargelcpvalues,
               Seqpos maxbranchdepth,
               const DefinedSeqpos *longest,
               GtError *err)
{
  FILE *prjfp;
  bool haserr = false;

  gt_error_check(err);
  prjfp = gt_fa_fopen_filename_with_suffix(indexname,PROJECTFILESUFFIX,
                                           "wb",err);
  if (prjfp == NULL)
  {
    haserr = true;
  }
  if (!haserr)
  {
    showprjinfo(prjfp,
                readmode,
                encseq,
                prefixlength,
                maxdepth,
                numoflargelcpvalues,
                maxbranchdepth,
                longest);
    gt_fa_xfclose(prjfp);
  }
  return haserr ? -1 : 0;
}
