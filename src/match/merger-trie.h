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

#ifndef MERGER_TRIE_H
#define MERGER_TRIE_H

#include "core/arraydef.h"
#include "seqpos-def.h"
#include "encodedsequence.h"

typedef struct
{
  unsigned int idx;
  Seqpos startpos;
#ifdef WITHTRIEIDENT
  uint64_t  ident;
#endif
} Suffixinfo;

typedef struct Mergertrienode
{
  Suffixinfo suffixinfo;
  struct Mergertrienode *firstchild,
                        *rightsibling,
                        *parent;
  Seqpos depth;
} Mergertrienode;

typedef Mergertrienode * Mergertrienodeptr;

GT_DECLAREARRAYSTRUCT(Mergertrienodeptr);

typedef struct
{
  GtEncodedsequence *encseqptr;
  Readmode readmode;
} Encseqreadinfo;

typedef struct
{
  Encseqreadinfo *encseqreadinfo;
  Mergertrienode *nodetable,
           *root;
  Mergertrienodeptr *unusedMergertrienodes;
  unsigned int numofindexes,
               nextunused,
               allocatedMergertrienode,
               nextfreeMergertrienode;
} Mergertrierep;

void mergertrie_show(const Mergertrierep *trierep,
                     const GtUchar *characters);

void mergertrie_check(Mergertrierep *trierep,unsigned int numberofleaves,
                     unsigned int maxleafnum,GtError *err);

void mergertrie_showallnoderelations(const Mergertrienode *node);

void mergertrie_insertsuffix(Mergertrierep *trierep,
                             Mergertrienode *node,
                             Suffixinfo *suffixinfo);

Mergertrienode *mergertrie_findsmallestnode(const Mergertrierep *trierep);

void mergertrie_deletesmallestpath(Mergertrienode *smallest,
                                   Mergertrierep *trierep);

void mergertrie_initnodetable(Mergertrierep *trierep,Seqpos numofsuffixes,
                              unsigned int numofindexes);

void mergertrie_delete(Mergertrierep *trierep);

#endif
