/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using RANDOM_init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

/*
*---------------------------------------------------------------------
*   INCLUDE FILES
*---------------------------------------------------------------------
*/

#include "random.h"

/*
*---------------------------------------------------------------------
*
*   INTERFACE (visible from other modules)
*
*---------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------
*   Function prototypes
*---------------------------------------------------------------------
*/

/* initializes mt19937_mt[RANDOM_mt19937_N] with a seed */

void RANDOM_init_genrand (unsigned long int s);

/* initialize by an array with array-length    */
/* init_key is the array for initializing keys */
/* key_length is its length                    */
/* slight change for C++, 2004/2/26            */

void RANDOM_init_by_array(unsigned long init_key[], int key_length);

unsigned long int RANDOM_genrand_int32 (void);  /* random number on [0,0xffffffff]-interval      */
long int          RANDOM_genrand_int31 (void);  /* random number on [0,0x7fffffff]-interval      */
double            RANDOM_genrand_real1 (void);  /* random number on [0,1]-real-interval          */
double            RANDOM_genrand_real2 (void);  /* random number on [0,1)-real-interval          */
double            RANDOM_genrand_real3 (void);  /* random number on (0,1)-real-interval          */
double            RANDOM_genrand_res53 (void);  /* random number on [0,1) with 53-bit resolution */

/*
*---------------------------------------------------------------------
*
*   IMPLEMENTATION (not visible from other modules)
*
*---------------------------------------------------------------------
*/

/* the array for the state vector */
static unsigned long int mt19937_mt[RANDOM_mt19937_N];

/* mt19937_mti==RANDOM_mt19937_N+1 means mt19937_mt[RANDOM_mt19937_N] is not initialized */
static int mt19937_mti=RANDOM_mt19937_N+1;

void RANDOM_init_genrand(unsigned long int s)
{
    mt19937_mt[0]= s & 0xffffffffUL;
    for (mt19937_mti=1; mt19937_mti<RANDOM_mt19937_N; mt19937_mti++) {
        mt19937_mt[mt19937_mti] =
	    (1812433253UL * (mt19937_mt[mt19937_mti-1] ^ (mt19937_mt[mt19937_mti-1] >> 30)) + mt19937_mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt19937_mt[].                */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt19937_mt[mt19937_mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

void RANDOM_init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    RANDOM_init_genrand(19650218UL);
    i=1; j=0;
    k = (RANDOM_mt19937_N>key_length ? RANDOM_mt19937_N : key_length);
    for (; k; k--) {
        mt19937_mt[i] = (mt19937_mt[i] ^ ((mt19937_mt[i-1] ^ (mt19937_mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt19937_mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=RANDOM_mt19937_N) { mt19937_mt[0] = mt19937_mt[RANDOM_mt19937_N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=RANDOM_mt19937_N-1; k; k--) {
        mt19937_mt[i] = (mt19937_mt[i] ^ ((mt19937_mt[i-1] ^ (mt19937_mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt19937_mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=RANDOM_mt19937_N) { mt19937_mt[0] = mt19937_mt[RANDOM_mt19937_N-1]; i=1; }
    }

    mt19937_mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
}

unsigned long RANDOM_genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, RANDOM_mt19937_MATRIX_A};
    /* mag01[x] = x * RANDOM_mt19937_MATRIX_A  for x=0,1 */

    if (mt19937_mti >= RANDOM_mt19937_N) { /* generate RANDOM_mt19937_N words at one time */
        int kk;

        if (mt19937_mti == RANDOM_mt19937_N+1)   /* if RANDOM_init_genrand() has not been called, */
            RANDOM_init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<RANDOM_mt19937_N-RANDOM_mt19937_M;kk++) {
            y = (mt19937_mt[kk]&RANDOM_mt19937_UPPER_MASK)|(mt19937_mt[kk+1]&RANDOM_mt19937_LOWER_MASK);
            mt19937_mt[kk] = mt19937_mt[kk+RANDOM_mt19937_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<RANDOM_mt19937_N-1;kk++) {
            y = (mt19937_mt[kk]&RANDOM_mt19937_UPPER_MASK)|(mt19937_mt[kk+1]&RANDOM_mt19937_LOWER_MASK);
            mt19937_mt[kk] = mt19937_mt[kk+(RANDOM_mt19937_M-RANDOM_mt19937_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt19937_mt[RANDOM_mt19937_N-1]&RANDOM_mt19937_UPPER_MASK)|(mt19937_mt[0]&RANDOM_mt19937_LOWER_MASK);
        mt19937_mt[RANDOM_mt19937_N-1] = mt19937_mt[RANDOM_mt19937_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mt19937_mti = 0;
    }

    y = mt19937_mt[mt19937_mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

long RANDOM_genrand_int31(void)
{
    return (long)(RANDOM_genrand_int32()>>1);
}

double RANDOM_genrand_real1(void)
{
    return (double) RANDOM_genrand_int32()*(1.0/4294967295.0);
    /* divided by 2^32-1 */
}

double RANDOM_genrand_real2(void)
{
    return (double) RANDOM_genrand_int32()*(1.0/4294967296.0);
    /* divided by 2^32 */
}

double RANDOM_genrand_real3(void)
{
    return (((double)RANDOM_genrand_int32()) + 0.5)*(1.0/4294967296.0);
    /* divided by 2^32 */
}

double RANDOM_genrand_res53(void)
{
    unsigned long a=RANDOM_genrand_int32()>>5, b=RANDOM_genrand_int32()>>6;
    return ((double) a*67108864.0+b)*(1.0/9007199254740992.0);
}

/* These real versions are due to Isaku Wada, 2002/01/09 added */
