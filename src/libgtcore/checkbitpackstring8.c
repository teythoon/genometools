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
icmp(uint8_t a, uint8_t b)
{
  if(a > b)
    return 1;
  else if(a < b)
    return -1;
  else /* if(a == b) */
    return 0;
}

int
bitPackString8_unit_test(Env *env)
{
  BitElem *bitStore = NULL;
  uint8_t *randSrc = NULL; /*< create random ints here for input as bit
                                *  store */
  uint8_t *randCmp = NULL; /*< used for random ints read back */
  size_t i, numRnd;
  BitOffset offsetStart, offset;
  unsigned long seedval;
  int had_err = 0;
  {
    struct timeval seed;
    gettimeofday(&seed, NULL);
    srandom(seedval = seed.tv_sec + seed.tv_usec);
  }
  offset = offsetStart = random()%(sizeof(uint8_t) * CHAR_BIT);
  numRnd = random() % MAX_RND_NUMS + 1;
#ifdef VERBOSE_UNIT_TEST
  fprintf(stderr, "seedval = %lu, offset=%lu, numRnd=%lu\n", seedval,
          (long unsigned)offsetStart, (long unsigned)numRnd);
#endif /* VERBOSE_UNIT_TEST */
  {
    BitOffset numBits = sizeof(uint8_t) * CHAR_BIT * numRnd + offsetStart;
    ensure(had_err, (randSrc = env_ma_malloc(env, sizeof(uint8_t)*numRnd))
           && (bitStore = env_ma_malloc(env, bitElemsAllocSize(numBits)
                                  * sizeof(BitElem)))
           && (randCmp = env_ma_malloc(env, sizeof(uint8_t)*numRnd)));
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
#if 8 > 32 && LONG_BIT < 8
    uint8_t v = randSrc[i] = (uint8_t)random() << 32 | random();
#else /* 8 > 32 && LONG_BIT < 8 */
    uint8_t v = randSrc[i] = random();
#endif /* 8 > 32 && LONG_BIT < 8 */
    int bits = requiredUInt8Bits(v);
    bsStoreUInt8(bitStore, offset, bits, v);
    offset += bits;
  }
  offset = offsetStart;
  for(i = 0; i < numRnd; ++i)
  {
    uint8_t v = randSrc[i];
    int bits = requiredUInt8Bits(v);
    uint8_t r = bsGetUInt8(bitStore, offset, bits);
    ensure(had_err, r == v);
    if(had_err)
    {
#ifdef VERBOSE_UNIT_TEST
      fprintf(stderr, "bsStoreUInt8/bsGetUInt8: "
              "Expected %"PRIu8", got %"PRIu8", seed = %lu, i = %lu\n",
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
  fputs("bsStoreUInt8/bsGetUInt8: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  {
    uint8_t v0 = randSrc[0];
    int bits0 = requiredUInt8Bits(v0);
    uint8_t r0;
    offset = offsetStart;
    r0 = bsGetUInt8(bitStore, offset, bits0);
    for(i = 1; i < numRnd; ++i)
    {
      uint8_t v1 = randSrc[i];
      int bits1 = requiredUInt8Bits(v1);
      uint8_t r1 = bsGetUInt8(bitStore, offset + bits0, bits1);
      int result;
      ensure(had_err, r0 == v0 && r1 == v1);
      ensure(had_err, icmp(v0, v1) ==
             (result = bsCompare(bitStore, offset, bits0,
                                 bitStore, offset + bits0, bits1)));
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr, "bsCompare: "
                "Expected v0 %s v1, got v0 %s v1,\n for v0=%"PRIu8
                " and v1=%"PRIu8",\n"
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
    unsigned numBits = random()%(sizeof(uint8_t)*CHAR_BIT) + 1;
    uint8_t mask = ~(uint8_t)0;
    if(numBits < 8)
      mask = ~(mask << numBits);
    offset = offsetStart;
    bsStoreUniformUInt8Array(bitStore, offset, numBits, numRnd, randSrc);
    for(i = 0; i < numRnd; ++i)
    {
      uint8_t v = randSrc[i] & mask;
      uint8_t r = bsGetUInt8(bitStore, offset, numBits);
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr, "bsStoreUniformUInt8Array/bsGetUInt8: "
                "Expected %"PRIu8", got %"PRIu8",\n seed = %lu,"
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
    fputs("bsStoreUniformUInt8Array/bsGetUInt8: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
    bsGetUniformUInt8Array(bitStore, offset = offsetStart,
                               numBits, numRnd, randCmp);
    for(i = 0; i < numRnd; ++i)
    {
      uint8_t v = randSrc[i] & mask;
      uint8_t r = randCmp[i];
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformUInt8Array/bsGetUniformUInt8Array: "
                "Expected %"PRIu8", got %"PRIu8",\n seed = %lu,"
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
      uint8_t v = randSrc[0] & mask;
      uint8_t r;
      bsGetUniformUInt8Array(bitStore, offsetStart,
                            numBits, 1, &r);
      if(r != v)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformUInt8Array/bsGetUniformUInt8Array: "
                "Expected %"PRIu8", got %"PRIu8", seed = %lu,"
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
    fputs(": bsStoreUniformUInt8Array/bsGetUniformUInt8Array:"
          " passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  }
  /* int types */
  for(i = 0; i < numRnd; ++i)
  {
    int8_t v = (int8_t)randSrc[i];
    unsigned bits = requiredInt8Bits(v);
    bsStoreInt8(bitStore, offset, bits, v);
    offset += bits;
  }
  offset = offsetStart;
  for(i = 0; i < numRnd; ++i)
  {
    int8_t v = randSrc[i];
    unsigned bits = requiredInt8Bits(v);
    int8_t r = bsGetInt8(bitStore, offset, bits);
    ensure(had_err, r == v);
    if(had_err)
    {
#ifdef VERBOSE_UNIT_TEST
      fprintf(stderr, "bsStoreInt8/bsGetInt8: "
              "Expected %"PRId8", got %"PRId8",\n"
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
  fputs(": bsStoreInt8/bsGetInt8: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  {
    unsigned numBits = random()%(sizeof(int8_t)*CHAR_BIT) + 1;
    int8_t mask = ~(int8_t)0;
    if(numBits < 8)
      mask = ~(mask << numBits);
    offset = offsetStart;
    bsStoreUniformInt8Array(bitStore, offset, numBits, numRnd,
                                (int8_t *)randSrc);
    for(i = 0; i < numRnd; ++i)
    {
      int8_t m = (int8_t)1 << (numBits - 1);
      int8_t v = (int8_t)((randSrc[i] & mask) ^ m) - m;
      int8_t r = bsGetInt8(bitStore, offset, numBits);
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr, "bsStoreUniformInt8Array/bsGetInt8: "
                "Expected %"PRId8", got %"PRId8",\n"
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
    fputs("bsStoreUniformInt8Array/bsGetInt8: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
    bsGetUniformInt8Array(bitStore, offset = offsetStart,
                              numBits, numRnd, (int8_t *)randCmp);
    for(i = 0; i < numRnd; ++i)
    {
      int8_t m = (int8_t)1 << (numBits - 1);
      int8_t v = (int8_t)((randSrc[i] & mask) ^ m) - m;
      int8_t r = randCmp[i];
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformInt8Array/bsGetUniformInt8Array: "
                "Expected %"PRId8", got %"PRId8
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
      int8_t m = (int8_t)1 << (numBits - 1);
      int8_t v = (int8_t)((randSrc[0] & mask) ^ m) - m;
      int8_t r;
      bsGetUniformInt8Array(bitStore, offsetStart,
                                numBits, 1, &r);
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformInt8Array/bsGetUniformInt8Array: "
                "Expected %"PRId8", got %"PRId8
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
    fputs(": bsStoreUniformInt8Array/bsGetUniformInt8Array:"
          " passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  }
  env_ma_free(randSrc, env);
  env_ma_free(randCmp, env);
  env_ma_free(bitStore, env);
  return had_err;
}

