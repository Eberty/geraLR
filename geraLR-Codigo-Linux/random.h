/*                                           */
/* Make sure this file is not included twice */
/*                                           */

#ifndef _RANDOM_DOT_H_
#define _RANDOM_DOT_H_

/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using RANDOM_init_genrand(seed)
   or RANDOM_init_by_array(init_key, key_length).

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



/* Period parameters */

#define RANDOM_mt19937_N 624
#define RANDOM_mt19937_M 397
#define RANDOM_mt19937_MATRIX_A   0x9908b0dfUL   /* constant vector a         */
#define RANDOM_mt19937_UPPER_MASK 0x80000000UL   /* most significant w-r bits */
#define RANDOM_mt19937_LOWER_MASK 0x7fffffffUL   /* least significant r bits  */


/* initializes mt19937_mt[RANDOM_mt19937_N] with a seed */

extern void RANDOM_init_genrand (unsigned long int s);

/* initialize by an array with array-length    */
/* init_key is the array for initializing keys */
/* key_length is its length                    */
/* slight change for C++, 2004/2/26            */

extern void RANDOM_init_by_array(unsigned long init_key[], int key_length);

extern unsigned long int RANDOM_genrand_int32 (void);  /* random number on [0,0xffffffff]-interval      */
extern long int          RANDOM_genrand_int31 (void);  /* random number on [0,0x7fffffff]-interval      */
extern double            RANDOM_genrand_real1 (void);  /* random number on [0,1]-real-interval          */
extern double            RANDOM_genrand_real2 (void);  /* random number on [0,1)-real-interval          */
extern double            RANDOM_genrand_real3 (void);  /* random number on (0,1)-real-interval          */
extern double            RANDOM_genrand_res53 (void);  /* random number on [0,1) with 53-bit resolution */

#endif /* ifndef _RANDOM_DOT_H_ */
