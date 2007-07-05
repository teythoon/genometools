/*
** Copyright (C) 2007 Thomas Jahns <Thomas.Jahns@gmx.net>
**  
** See LICENSE file or http://genometools.org/license.html for license details.
** 
*/
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <time.h>
#include <sys/time.h>

#include <libgtcore/bitpackstring.h>
#include <libgtcore/env.h>
#include <libgtcore/ensure.h>

enum {
/*   MAX_RND_NUMS = 10, */
  MAX_RND_NUMS = 10000000,
};

static inline int
icmp(uint64_t a, uint64_t b)
{
  if(a > b)
    return 1;
  else if(a < b)
    return -1;
  else /* if(a == b) */
    return 0;
}

int
bitPackString64_unit_test(Env *env)
{
  BitElem *bitStore = NULL;
  uint64_t *randSrc = NULL; /*< create random ints here for input as bit
                                *  store */
  uint64_t *randCmp = NULL; /*< used for random ints read back */
  size_t i, numRnd;
  BitOffset offsetStart, offset;
  unsigned long seedval;
  int had_err = 0;
  {
    struct timeval seed;
    gettimeofday(&seed, NULL);
    srandom(seedval = seed.tv_sec + seed.tv_usec);
  }
  offset = offsetStart = random()%(sizeof(uint64_t) * CHAR_BIT);
  numRnd = random() % MAX_RND_NUMS + 1;
#ifdef VERBOSE_UNIT_TEST
  fprintf(stderr, "seedval = %lu, offset=%lu, numRnd=%lu\n", seedval,
          (long unsigned)offsetStart, (long unsigned)numRnd);
#endif /* VERBOSE_UNIT_TEST */
  {
    BitOffset numBits = sizeof(uint64_t) * CHAR_BIT * numRnd + offsetStart;
    ensure(had_err, (randSrc = env_ma_malloc(env, sizeof(uint64_t)*numRnd))
           && (bitStore = env_ma_malloc(env, bitElemsAllocSize(numBits)
                                  * sizeof(BitElem)))
           && (randCmp = env_ma_malloc(env, sizeof(uint64_t)*numRnd)));
  }
  if(had_err)
  {
    if(randSrc)
      env_ma_free(randSrc, env);
    if(randCmp)
      env_ma_free(randCmp, env);
    if(bitStore)
      env_ma_free(bitStore, env);
#ifdef VERBOSE_UNIT_TEST
    perror("Storage allocations failed");
#endif /* VERBOSE_UNIT_TEST */
    return had_err;
  }
  /* first test unsigned types */
  for(i = 0; i < numRnd; ++i)
  {
#if 64 > 32 && LONG_BIT < 64
    uint64_t v = randSrc[i] = (uint64_t)random() << 32 | random();
#else /* 64 > 32 && LONG_BIT < 64 */
    uint64_t v = randSrc[i] = random();
#endif /* 64 > 32 && LONG_BIT < 64 */
    int bits = requiredUInt64Bits(v);
    bsStoreUInt64(bitStore, offset, bits, v);
    offset += bits;
  }
  offset = offsetStart;
  for(i = 0; i < numRnd; ++i)
  {
    uint64_t v = randSrc[i];
    int bits = requiredUInt64Bits(v);
    uint64_t r = bsGetUInt64(bitStore, offset, bits);
    ensure(had_err, r == v);
    if(had_err)
    {
#ifdef VERBOSE_UNIT_TEST
      fprintf(stderr, "bsStoreUInt64/bsGetUInt64: "
              "Expected %"PRIu64", got %"PRIu64", seed = %lu, i = %lu\n",
              v, r, seedval, (unsigned long)i);
#endif /* VERBOSE_UNIT_TEST */
      env_ma_free(randSrc, env);
      env_ma_free(randCmp, env);
      env_ma_free(bitStore, env);
      return had_err;
    }
    offset += bits;
  }
#ifdef VERBOSE_UNIT_TEST
  fputs("bsStoreUInt64/bsGetUInt64: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  {
    uint64_t v0 = randSrc[0];
    int bits0 = requiredUInt64Bits(v0);
    uint64_t r0;
    offset = offsetStart;
    r0 = bsGetUInt64(bitStore, offset, bits0);
    for(i = 1; i < numRnd; ++i)
    {
      uint64_t v1 = randSrc[i];
      int bits1 = requiredUInt64Bits(v1);
      uint64_t r1 = bsGetUInt64(bitStore, offset + bits0, bits1);
      int result;
      ensure(had_err, r0 == v0 && r1 == v1);
      ensure(had_err, icmp(v0, v1) ==
             (result = bsCompare(bitStore, offset, bits0,
                                 bitStore, offset + bits0, bits1)));
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr, "bsCompare: "
                "Expected v0 %s v1, got v0 %s v1,\n for v0=%"PRIu64
                " and v1=%"PRIu64",\n"
                "seed = %lu, i = %lu, bits0=%u, bits1=%u\n",
                (v0 > v1?">":(v0 < v1?"<":"==")),
                (result > 0?">":(result < 0?"<":"==")), v0, v1,
                seedval, (unsigned long)i, bits0, bits1);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
      offset += bits0;
      bits0 = bits1;
      v0 = v1;
      r0 = r1;
    }
  }
#ifdef VERBOSE_UNIT_TEST
  fputs("bsCompare: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  {
    unsigned numBits = random()%(sizeof(uint64_t)*CHAR_BIT) + 1;
    uint64_t mask = ~(uint64_t)0;
    if(numBits < 64)
      mask = ~(mask << numBits);
    offset = offsetStart;
    bsStoreUniformUInt64Array(bitStore, offset, numBits, numRnd, randSrc);
    for(i = 0; i < numRnd; ++i)
    {
      uint64_t v = randSrc[i] & mask;
      uint64_t r = bsGetUInt64(bitStore, offset, numBits);
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr, "bsStoreUniformUInt64Array/bsGetUInt64: "
                "Expected %"PRIu64", got %"PRIu64",\n seed = %lu,"
                " i = %lu, bits=%u\n",
                v, r, seedval, (unsigned long)i, numBits);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
      offset += numBits;
    }
#ifdef VERBOSE_UNIT_TEST
    fputs("bsStoreUniformUInt64Array/bsGetUInt64: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
    bsGetUniformUInt64Array(bitStore, offset = offsetStart,
                               numBits, numRnd, randCmp);
    for(i = 0; i < numRnd; ++i)
    {
      uint64_t v = randSrc[i] & mask;
      uint64_t r = randCmp[i];
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformUInt64Array/bsGetUniformUInt64Array: "
                "Expected %"PRIu64", got %"PRIu64",\n seed = %lu,"
                " i = %lu, bits=%u\n",
                v, r, seedval, (unsigned long)i, numBits);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
    }
    {
      uint64_t v = randSrc[0] & mask;
      uint64_t r;
      bsGetUniformUInt64Array(bitStore, offsetStart,
                            numBits, 1, &r);
      if(r != v)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformUInt64Array/bsGetUniformUInt64Array: "
                "Expected %"PRIu64", got %"PRIu64", seed = %lu,"
                " one value extraction\n",
                v, r, seedval);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
    }    
#ifdef VERBOSE_UNIT_TEST
    fputs(": bsStoreUniformUInt64Array/bsGetUniformUInt64Array:"
          " passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  }
  /* int types */
  for(i = 0; i < numRnd; ++i)
  {
    int64_t v = (int64_t)randSrc[i];
    unsigned bits = requiredInt64Bits(v);
    bsStoreInt64(bitStore, offset, bits, v);
    offset += bits;
  }
  offset = offsetStart;
  for(i = 0; i < numRnd; ++i)
  {
    int64_t v = randSrc[i];
    unsigned bits = requiredInt64Bits(v);
    int64_t r = bsGetInt64(bitStore, offset, bits);
    ensure(had_err, r == v);
    if(had_err)
    {
#ifdef VERBOSE_UNIT_TEST
      fprintf(stderr, "bsStoreInt64/bsGetInt64: "
              "Expected %"PRId64", got %"PRId64",\n"
              "seed = %lu, i = %lu, bits=%u\n",
              v, r, seedval, (unsigned long)i, bits);
#endif /* VERBOSE_UNIT_TEST */
      env_ma_free(randSrc, env);
      env_ma_free(randCmp, env);
      env_ma_free(bitStore, env);
      return had_err;
    }
    offset += bits;
  }
#ifdef VERBOSE_UNIT_TEST
  fputs(": bsStoreInt64/bsGetInt64: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  {
    unsigned numBits = random()%(sizeof(int64_t)*CHAR_BIT) + 1;
    int64_t mask = ~(int64_t)0;
    if(numBits < 64)
      mask = ~(mask << numBits);
    offset = offsetStart;
    bsStoreUniformInt64Array(bitStore, offset, numBits, numRnd,
                                (int64_t *)randSrc);
    for(i = 0; i < numRnd; ++i)
    {
      int64_t m = (int64_t)1 << (numBits - 1);
      int64_t v = (int64_t)((randSrc[i] & mask) ^ m) - m;
      int64_t r = bsGetInt64(bitStore, offset, numBits);
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr, "bsStoreUniformInt64Array/bsGetInt64: "
                "Expected %"PRId64", got %"PRId64",\n"
                "seed = %lu, i = %lu, numBits=%u\n",
                v, r, seedval, (unsigned long)i, numBits);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
      offset += numBits;
    }
#ifdef VERBOSE_UNIT_TEST
    fputs("bsStoreUniformInt64Array/bsGetInt64: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
    bsGetUniformInt64Array(bitStore, offset = offsetStart,
                              numBits, numRnd, (int64_t *)randCmp);
    for(i = 0; i < numRnd; ++i)
    {
      int64_t m = (int64_t)1 << (numBits - 1);
      int64_t v = (int64_t)((randSrc[i] & mask) ^ m) - m;
      int64_t r = randCmp[i];
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformInt64Array/bsGetUniformInt64Array: "
                "Expected %"PRId64", got %"PRId64
                ", seed = %lu, i = %lu\n",
                v, r, seedval, (unsigned long)i);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
    }
    {
      int64_t m = (int64_t)1 << (numBits - 1);
      int64_t v = (int64_t)((randSrc[0] & mask) ^ m) - m;
      int64_t r;
      bsGetUniformInt64Array(bitStore, offsetStart,
                                numBits, 1, &r);
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformInt64Array/bsGetUniformInt64Array: "
                "Expected %"PRId64", got %"PRId64
                ", seed = %lu, one value extraction\n",
                v, r, seedval);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
    }    
#ifdef VERBOSE_UNIT_TEST
    fputs(": bsStoreUniformInt64Array/bsGetUniformInt64Array:"
          " passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  }
  env_ma_free(randSrc, env);
  env_ma_free(randCmp, env);
  env_ma_free(bitStore, env);
  return had_err;
}

