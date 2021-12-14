/* Copyright (c) 2009-2013 Kyle Gorman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the 
 * "Software"), to deal in the Software without restriction, including 
 * without limitation the rights to use, copy, modify, merge, publish, 
 * distribute, sublicense, and/or sell copies of the Software, and to 
 * permit persons to whom the Software is furnished to do so, subject to 
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * vector.h: header for vector.c
 * Kyle Gorman <gormanky@ohsu.edu>
 */

#ifndef VECTOR_H
#define VECTOR_H

#if 1
namespace sptk {
namespace swipe {
#endif

// vector stuff
typedef struct { 
    int x; 
    double *v; 
} vector;

vector                         makev(int);
vector                         zerov(int);
vector                         onesv(int);
vector                         nansv(int);
vector                         copyv(vector);

int                            maxv(vector);
int                            minv(vector);
int                            bisectv(vector, double);
int                            bilookv(vector, double, int);

void                           freev(vector);
void                           printv(vector);

// intvector stuff
typedef struct { 
    int x; 
    int *v; 
} intvector;

intvector                      makeiv(int);
intvector                      zeroiv(int);
intvector                      onesiv(int);
intvector                      copyiv(intvector);

vector                         iv2v(intvector);

int                            maxiv(intvector);
int                            miniv(intvector);
int                            bisectiv(intvector, int);
int                            bilookiv(intvector, int, int);

void                           freeiv(intvector);
void                           printiv(intvector);

// matrix stuff
typedef struct { 
    int x; 
    int y; 
    double **m; 
} matrix;

matrix                         makem(int, int);
matrix                         zerom(int, int); 
matrix                         onesm(int, int); 
matrix                         nansm(int, int);
matrix                         copym(matrix);
void                           freem(matrix);
void                           printm(matrix);

// intmatrix stuff
typedef struct { 
    int x; 
    int y; 
    int **m; 
} intmatrix;

intmatrix                      makeim(int, int);
intmatrix                      zeroim(int, int); 
intmatrix                      onesim(int, int); 
intmatrix                      copyim(intmatrix);

matrix                         im2m(intmatrix); // cast

void                           freeim(intmatrix);
void                           printim(intmatrix);

// prime sieve
#define NP                     0
#define PR                     1

#define PRIME(x)               (x == 1)

int                            sieve(intvector);
intvector                      primes(int);

// cubic spline
#define YP1                    2.
#define YPN                    2.

vector                         spline(vector, vector);
double                         splinv(vector, vector, vector, double, int);

// polynomial fitting
vector                         polyfit(vector, vector, int);
double                         polyval(vector, double);

#if 1
}  // namespace swipe
}  // namespace sptk
#endif

#endif
