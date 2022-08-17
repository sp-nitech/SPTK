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
 * swipe.c: primary functions
 * Kyle Gorman <gormanky@ohsu.edu>
 */

#define VNUM    1.5 // current version

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if 0
#include <unistd.h>
#endif
#include <limits.h>
#include <stdbool.h>

#if 0
#include <fftw3.h>   // http://www.fftw.org/
#include <sndfile.h> // http://www.mega-nerd.com/libsndfile/
#else
#include "swipe.h"
#include "SPTK/math/real_valued_fast_fourier_transform.h"
#endif

#include "vector.h"  // comes with release

#define NOK      0

#define DERBS    .1 
#define POLYV    .0013028 //  1 / 12 / 64 = 1 / 768
#define DLOG2P   .0104167 // 1/96

// feel free to change these defaults
#define ST       .3  
#define DT       .001
#define MIN      100.
#define MAX      600.

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#ifndef NAN
    #define NAN sqrt(-1.)
#endif

#if 1
namespace sptk {
namespace swipe {
#endif

#ifndef isnan
int isnan(double x) { 
    return(x != x);
}
#endif

#ifndef log2
// a base-2 log function
double log2(double x) { 
    return log(x) / log(2.);
}
#endif

#ifndef round
// rounds a double to the nearest integer value
double round(double x) { 
    return(x >= 0. ? floor(x + .5) : floor(x - .5));
}
#endif

// converts from hertz to Mel frequency
double hz2mel(double hz) { 
    return(1127.01048 * log(1. + hz / 700.));
}

// converts from hertz to ERBs
double hz2erb(double hz) { 
    return(21.4 * log10(1. + hz / 229.));
}

// converts from ERBs to hertz 
double erb2hz(double erb) { 
    return((pow(10, erb / 21.4) - 1.) * 229.);
}

// a silly function that treats NaNs as 0.
double fixnan(double x) { 
    return(isnan(x) ? 0. : x);
}

// a helper function for loudness() for individual fft slices
#if 0
void La(matrix L, vector f, vector fERBs, fftw_plan plan, 
                            fftw_complex* fo, int w2, int hi, int i) {
    int j;
    fftw_execute(plan);
    vector a = makev(w2);
    for (j = 0; j < w2; j++) // this iterates over only the first half
        a.v[j] = sqrt(fo[j][0] * fo[j][0] + fo[j][1] * fo[j][1]);
#else
void La(matrix L, vector f, vector fERBs,
                  const sptk::RealValuedFastFourierTransform& plan,
                  const std::vector<double>& fi,
                  std::vector<std::vector<double> >* fo,
                  sptk::RealValuedFastFourierTransform::Buffer* buffer,
                  int w2, int hi, int i) {
    int j;
    if (!plan.Run(fi, &((*fo)[0]), &((*fo)[1]), buffer))
        return;
    double* fo0 = &(((*fo)[0])[0]);
    double* fo1 = &(((*fo)[1])[0]);
    vector a = makev(w2);
    for (j = 0; j < w2; j++) // this iterates over only the first half
        a.v[j] = sqrt(fo0[j] * fo0[j] + fo1[j] * fo1[j]);
#endif
    vector a2 = spline(f, a); // a2 is now the result of the cubic spline
    L.m[i][0] = fixnan(sqrt(splinv(f, a, a2, fERBs.v[0], hi)));
    for (j = 1; j < L.y; j++) { // perform a bisection query at ERB intvls
        hi = bilookv(f, fERBs.v[j], hi); 
        L.m[i][j] = fixnan(sqrt(splinv(f, a, a2, fERBs.v[j], hi)));
    }
    freev(a); 
    freev(a2);
}

// a function for populating the loudness matrix with a signal x
matrix loudness(vector x, vector fERBs, double nyquist, int w, int w2) {
    int i, j, hi; 
    int offset = 0;
    double td = nyquist / w2; // this is equivalent to fstep
    // testing showed this configuration of fftw to be fastest
#if 0
    double* fi = fftw_malloc(sizeof(double) * w); 
    fftw_complex* fo = fftw_malloc(sizeof(fftw_complex) * w);
    fftw_plan plan = fftw_plan_dft_r2c_1d(w, fi, fo, FFTW_ESTIMATE); 
#else
    std::vector<double> fi(w); 
    std::vector<std::vector<double> > fo(2, std::vector<double>(w));
    sptk::RealValuedFastFourierTransform plan(w2 * 2 - 1, w2 * 2);
    sptk::RealValuedFastFourierTransform::Buffer buffer;
#endif
    vector hann = makev(w); // this defines the Hann[ing] window
    for (i = 0; i < w; i++) 
        hann.v[i] = .5 - (.5 * cos(2. * M_PI * ((double) i / w)));
    vector f = makev(w2);
    for (i = 0; i < w2; i++) 
        f.v[i] = i * td;
    hi = bisectv(f, fERBs.v[0]); // all calls to La() will begin here
    matrix L = makem(ceil((double) x.x / w2) + 1, fERBs.x); 
    for (j = 0; j < w2; j++) // left boundary case
        fi[j] = 0.; // more explicitly, 0. * hann.v[j]
    for (/* j = w2 */; j < w; j++) 
        fi[j] = x.v[j - w2] * hann.v[j];
#if 0
    La(L, f, fERBs, plan, fo, w2, hi, 0); 
#else
    La(L, f, fERBs, plan, fi, &fo, &buffer, w2, hi, 0); 
#endif
    for (i = 1; i < L.x - 2; i++) { 
        for (j = 0; j < w; j++) 
            fi[j] = x.v[j + offset] * hann.v[j];
#if 0
        La(L, f, fERBs, plan, fo, w2, hi, i); 
#else
        La(L, f, fERBs, plan, fi, &fo, &buffer, w2, hi, i); 
#endif
        offset += w2;
    }
    for (/* i = L.x - 2; */; i < L.x; i++) { // right two boundary cases
        for (j = 0; j < x.x - offset; j++) // this dies at x.x + w2
            fi[j] = x.v[j + offset] * hann.v[j];
        for (/* j = x.x - offset */; j < w; j++) 
            fi[j] = 0.; // once again, 0. * hann.v[j] 
#if 0
        La(L, f, fERBs, plan, fo, w2, hi, i);
#else
        La(L, f, fERBs, plan, fi, &fo, &buffer, w2, hi, i);
#endif
        offset += w2;
    } // now L is fully valued
    freev(hann);
    freev(f);
    // L must now be normalized
    for (i = 0; i < L.x; i++) { 
        td = 0.; // td is the value of the normalization factor
        for (j = 0; j < L.y; j++) 
            td += L.m[i][j] * L.m[i][j];
        if (td != 0.) { // catches zero-division
            td = sqrt(td);
            for (j = 0; j < L.y; j++) 
                L.m[i][j] /= td;
        } // otherwise, it is already 0.
    } 
#if 0
    fftw_destroy_plan(plan); 
    fftw_free(fi); 
    fftw_free(fo); 
#endif
    return(L);
}

// populates the strength matrix using the loudness matrix
void Sadd(matrix S, matrix L, vector fERBs, vector pci, vector mu, 
                                            intvector ps, double dt, 
                                            double nyquist2, int lo, 
                                            int psz, int w2) {
    int i, j, k;
    double t = 0.;
    double tp = 0.;
    double td;
    double dtp = w2 / nyquist2;
    matrix Slocal = zerom(psz, L.x);
    for (i = 0; i < Slocal.x; i++) {
        vector q = makev(fERBs.x);
        for (j = 0; j < q.x; j++) q.v[j] = fERBs.v[j] / pci.v[i];
        vector kernel = zerov(fERBs.x); // a zero-filled kernel vector
        for (j = 0; j < ps.x; j++) {
            if PRIME(ps.v[j]) {
                for (k = 0; k < kernel.x; k++) {
                    td = fabs(q.v[k] - j - 1.); 
                    if (td < .25) // peaks
                        kernel.v[k] = cos(2. * M_PI * q.v[k]);
                    else if (td < .75)  // valleys
                        kernel.v[k] += cos(2. * M_PI * q.v[k]) / 2.;
                }
            }
        }
        freev(q);
        td = 0.; 
        for (j = 0; j < kernel.x; j++) {
            kernel.v[j] *= sqrt(1. / fERBs.v[j]); // applying the envelope
            if (kernel.v[j] > 0.) 
                td += kernel.v[j] * kernel.v[j];
        }
        td = sqrt(td); // now, td is the p=2 norm factor
        for (j = 0; j < kernel.x; j++) // normalize the kernel
            kernel.v[j] /= td;
        for (j = 0; j < L.x; j++) { 
            for (k = 0; k < L.y; k++) 
                Slocal.m[i][j] += kernel.v[k] * L.m[j][k]; // i.e, kernel' * L
        }
        freev(kernel);
    } // Slocal is filled out; time to interpolate
    k = 0; 
    for (j = 0; j < S.y; j++) { // determine the interpolation params 
        td = t - tp; 
        while (td >= 0.) {
            k++;
            tp += dtp;
            td -= dtp;
        } // td now equals the time difference
        for (i = 0; i < psz; i++) {
            S.m[lo + i][j] += (Slocal.m[i][k] + (td * (Slocal.m[i][k] -
                                    Slocal.m[i][k - 1])) / dtp) * mu.v[i];
        }
        t += dt;
    }
    freem(Slocal);
}

// helper function for populating the strength matrix on left boundary
void Sfirst(matrix S, vector x, vector pc, vector fERBs, vector d, 
                                           intvector ws, intvector ps, 
                                           double nyquist, double nyquist2,
                                           double dt, int n) {
    int i; 
    int w2 = ws.v[n] / 2;
    matrix L = loudness(x, fERBs, nyquist, ws.v[n], w2);
    int lo = 0; // the start of Sfirst-specific code
    int hi = bisectv(d, 2.);
    int psz = hi - lo;
    vector mu = makev(psz);
    vector pci = makev(psz);
    for (i = 0; i < hi; i++) {
        pci.v[i] = pc.v[i];
        mu.v[i] = 1. - fabs(d.v[i] - 1.);
    } // end of Sfirst-specific code
    Sadd(S, L, fERBs, pci, mu, ps, dt, nyquist2, lo, psz, w2); 
    freem(L);
    freev(mu);
    freev(pci); 
}

// generic helper function for populating the strength matrix
void Snth(matrix S, vector x, vector pc, vector fERBs, vector d,
                              intvector ws, intvector ps, double nyquist, 
                              double nyquist2, double dt, int n) {
    int i;
    int w2 = ws.v[n] / 2;
    matrix L = loudness(x, fERBs, nyquist, ws.v[n], w2);
    int lo = bisectv(d, n); // start of Snth-specific code
    int hi = bisectv(d, n + 2);
    int psz = hi - lo;
    vector mu = makev(psz);
    vector pci = makev(psz);
    int ti = 0;
    for (i = lo; i < hi; i++) {
        pci.v[ti] = pc.v[i];
        mu.v[ti] = 1. - fabs(d.v[i] - (n + 1));
        ti++;
    } // end of Snth-specific code
    Sadd(S, L, fERBs, pci, mu, ps, dt, nyquist2, lo, psz, w2); 
    freem(L);
    freev(mu);
    freev(pci); 
}

// helper function for populating the strength matrix from the right boundary
void Slast(matrix S, vector x, vector pc, vector fERBs, vector d, 
                                          intvector ws, intvector ps, 
                                          double nyquist, double nyquist2, 
                                          double dt, int n) {
    int i;
    int w2 = ws.v[n] / 2;
    matrix L = loudness(x, fERBs, nyquist, ws.v[n], w2);
    int lo = bisectv(d, n); // start of Slast-specific code
    int hi = d.x;
    int psz = hi - lo;
    vector mu = makev(psz);
    vector pci = makev(psz);
    int ti = 0;
    for (i = lo; i < hi; i++) {
        pci.v[ti] = pc.v[i];
        mu.v[ti] = 1. - fabs(d.v[i] - (n + 1));
        ti++;
    } // end of Slast-specific code
    Sadd(S, L, fERBs, pci, mu, ps, dt, nyquist2, lo, psz, w2); 
    freem(L);
    freev(mu);
    freev(pci); 
}

// performs polynomial tuning on the strength matrix to determine the pitch
vector pitch(matrix S, vector pc, double st) {
    int i, j;
    int maxi = -1;
    int search = (int) round((log2(pc.v[2]) - log2(pc.v[0])) / POLYV + 1.);
    double nftc, maxv, log2pc;
    double tc2 = 1. / pc.v[1];
    vector coefs;
    vector s = makev(3);
    vector ntc = makev(3);
    ntc.v[0] = ((1. / pc.v[0]) / tc2 - 1.) * 2. * M_PI; 
    ntc.v[1] = (tc2 / tc2 - 1.) * 2. * M_PI; 
    ntc.v[2] = ((1. / pc.v[2]) / tc2 - 1.) * 2. * M_PI;
    vector p = makev(S.y);  
    for (j = 0; j < S.y; j++) {
        maxv = SHRT_MIN;  
        for (i = 0; i < S.x; i++) {
            if (S.m[i][j] > maxv) {
                maxv = S.m[i][j];
                maxi = i;
            }
        }
        if (maxv > st) { // make sure it's big enough
            if (maxi == 0 || maxi == S.x - 1) p.v[j] = pc.v[0];
            else { // general case
                tc2 = 1. / pc.v[maxi];
                log2pc = log2(pc.v[maxi - 1]); 
                s.v[0] = S.m[maxi - 1][j];
                s.v[1] = S.m[maxi][j];
                s.v[2] = S.m[maxi + 1][j]; 
                coefs = polyfit(ntc, s, 2); 
                maxv = SHRT_MIN; 
                for (i = 0; i < search; i++) { // check the nftc space
                    nftc = polyval(coefs, ((1. / pow(2, i * POLYV + 
                                   log2pc)) / tc2 - 1) * 2 * M_PI);
                    if (nftc > maxv) {
                        maxv = nftc;
                        maxi = i;
                    }
                } // now we've got the pitch numbers we need
                freev(coefs);
                p.v[j] = pow(2, log2pc + (maxi * POLYV));
            }
        }
        else 
#if 0
            p.v[j] = NAN;
#else
            p.v[j] = 0.0;
#endif
    }
    freev(ntc);
    freev(s);
    return(p);
}

// primary utility function for each pitch extraction
#if 0
vector swipe(int fid, double min, double max, double st, double dt) {
#else
vector swipe(const std::vector<double>& waveform, double samplerate, double min,
             double max, double st, double dt) {
#endif
    int i; 
    double td = 0.;
#if 0
    SF_INFO info;
    SNDFILE* source = sf_open_fd(fid, SFM_READ, &info, true);
    if (source == NULL || info.sections < 1) 
        return(makev(0)); 
    double nyquist = info.samplerate / 2.;
    double nyquist2 = info.samplerate;
    double nyquist16 = info.samplerate * 8.;
#else
    double nyquist = samplerate / 2.;
    double nyquist2 = samplerate;
    double nyquist16 = samplerate * 8.;
#endif
    if (max > nyquist) {
        max = nyquist;
        fprintf(stderr, "Max pitch exceeds Nyquist frequency...");
        fprintf(stderr, "max pitch set to %.2f Hz.\n", max);
    }
    if (dt > nyquist2) {
        dt = nyquist2;
        fprintf(stderr, "Timestep > SR...timestep set to %f.\n", nyquist2);
    }
    intvector ws = makeiv(round(log2((nyquist16) / min) -  
                                log2((nyquist16) / max)) + 1); 
    for (i = 0; i < ws.x; i++)
        ws.v[i] = pow(2, round(log2(nyquist16 / min))) / pow(2, i);
    vector pc = makev(ceil((log2(max) - log2(min)) / DLOG2P));
    vector d = makev(pc.x);
    for (i = pc.x - 1; i >= 0; i--) { 
        td = log2(min) + (i * DLOG2P);
        pc.v[i] = pow(2, td);
        d.v[i] = 1. + td - log2(nyquist16 / ws.v[0]); 
    } // td now equals log2(min)
#if 0
    vector x = makev((int) info.frames); // read in the signal
    sf_read_double(source, x.v, x.x);
    sf_close(source); // takes wavf with it, too
#else
    double n = 1. / 32768.;
    int frames = static_cast<int>(waveform.size());
    vector x = makev(frames);
    for (i = 0; i < frames; i++)
        x.v[i] = waveform[i] * n;
#endif
    vector fERBs = makev(ceil((hz2erb(nyquist) - 
                               hz2erb(pow(2, td) / 4)) / DERBS));
    td = hz2erb(min / 4.);
    for (i = 0; i < fERBs.x; i++) 
        fERBs.v[i] = erb2hz(td + (i * DERBS));
    intvector ps = onesiv(floor(fERBs.v[fERBs.x - 1] / pc.v[0] - .75));
    sieve(ps);
    ps.v[0] = PR; // hack to make 1 "act" prime...don't ask
    matrix S = zerom(pc.x, ceil(((double) x.x / nyquist2) / dt));
    Sfirst(S, x, pc, fERBs, d, ws, ps, nyquist, nyquist2, dt, 0); 
    for (i = 1; i < ws.x - 1; i++) // S is updated inline here
        Snth(S, x, pc, fERBs, d, ws, ps, nyquist, nyquist2, dt, i);
    // i is now (ws.x - 1)
    Slast(S, x, pc, fERBs, d, ws, ps, nyquist, nyquist2, dt, i);
    freev(fERBs); 
    freeiv(ws);
    freeiv(ps);
    freev(d);  
    freev(x);
    vector p = pitch(S, pc, st); // find pitch using strength matrix
    freev(pc);
    freem(S);
    return(p);
}

#if 0
// a Python version of the call
vector pyswipe(char wav[], double min, double max, double st, double dt) {
    return swipe(fileno(fopen(wav, "r")), min, max, st, dt);
}

// function for printing the pitch vector returned by swipe()
void printp(vector p, int fid, double dt, int mel, int vlo) {
    int i;
    double t = 0.; 
    FILE* sink = fdopen(fid, "w");
    if (mel) {
        if (vlo) {
            for (i = 0; i < p.x; i++) {
                fprintf(sink, "%4.4f %5.4f\n", t, hz2mel(p.v[i])); 
                t += dt;
            }
        }
        else { // Default case
            for (i = 0; i < p.x; i++) {
                if (!isnan(p.v[i]))
                    fprintf(sink, "%4.4f %5.4f\n", t, hz2mel(p.v[i])); 
                t += dt;
            }
        }
    }
    else {
        if (vlo) {
            for (i = 0; i < p.x; i++) {
                fprintf(sink, "%4.4f %5.4f\n", t, p.v[i]); 
                t += dt;
            }
        }
        else { 
            for (i = 0; i < p.x; i++) {
                if (!isnan(p.v[i])) 
                    fprintf(sink, "%4.4f %5.4f\n", t, p.v[i]);
                t += dt;
            }
        }
    } 
}

// main method, interfacing with user arguments
int main(int argc, char* argv[]) {
    char output[] = "OUTPUT:\npitch_0\ttime_0\npitch_1\ttime_1\n...\t...\
    \npitch_N\ttime_N\n\n"; 
    char header[] = "\nSWIPE' pitch tracker, implemented by Kyle Gorman \
<gormanky@ohsu.edu>, \nbased on: A. Camacho. 2007. A sawtooth \
waveform inspired pitch estimator\nfor speech and music. Doctoral \
dissertation, U of Florida.\n\n\
More information: <http://ling.upenn.edu/~kgorman/C/swipe/>\n\n";
    char synops[] = "SYNPOSIS:\n\n\
swipe [-i FILE] [-o FILE] [-b LIST] [-r MIN:MAX] [-s TS] [-t DT] [-mnhv]\n\
\nFLAG:\t\tDESCRIPTION:\t\t\t\t\tDEFAULT:\n\n\
-i FILE\t\tinput file\t\t\t\t\t<STDIN>\n\
-o FILE\t\toutput file\t\t\t\t\t<STDOUT>\n\
-b LIST\t\tbatch mode [LIST is a file containing\n\
\t\tone \"INPUT OUTPUT\" pair per line]\n\n\
-r MIN:MAX\tpitchrange in Hertz\t\t\t\t100:600\n\
-s TIMESTEP\ttimestep in seconds\t\t\t\t0.001\n\
-t THRESHOLD\tstrength threshold [0 <= x <= 1]\t\t0.300\n\n\
-m\t\tOutput Mel pitch\t\t\t\tno\n\
-n\t\tDon't output voiceless frames\t\t\tno\n\
-h\t\tDisplay this message, then quit\n\
-v\t\tDisplay version number, then quit\n\n";
    // all set by #defines
    double st = ST; 
    double dt = DT;
    bool vlo = true;
    bool mel = false;
    double min = MIN;
    double max = MAX; 
    int ch;
    FILE* batch = NULL; // not going to be read that way,
    // some, but not all, compilers initialize char*s to be "\0"
    char* wav = "\0"; 
    char* out = "\0";
    int needed;
    while ((ch = getopt(argc, argv, "i:o:r:s:t:b:mnhv")) != -1) {
        switch(ch) {
            case 'b':
                batch = fopen(optarg, "r");
                break;
            case 'i':
                needed = (int) strlen(optarg);
                if (needed > FILENAME_MAX) {
                    fprintf(stderr, "Filename too long, aborting.\n");
                    exit(EXIT_FAILURE);
                }
                wav = (char *) malloc(sizeof(char) * needed);
                strcpy(wav, optarg);
                break; 
            case 'o':
                needed = (int) strlen(optarg);
                if (needed > FILENAME_MAX) {
                    fprintf(stderr, "Filename too long, aborting.\n");
                    exit(EXIT_FAILURE);
                }
                out = (char *) malloc(sizeof(char) * needed);
                strcpy(out, optarg);
                break;
            case 'r':
                min = atof(strtok(optarg, ":"));
                max = atof(strtok(NULL, ":")); 
                break;
            case 't':
                st = atof(optarg);
                break;
            case 's':
                dt = atof(optarg);
                break;
            case 'm':
                mel = true; 
                break;
            case 'n':
                vlo = false; 
                break;
            case 'h':
                fprintf(stderr, "%s", header); 
                fprintf(stderr, "%s", synops); 
                fprintf(stderr, "%s", output);
                exit(EXIT_SUCCESS);
            case 'v':
                fprintf(stderr, "This is SWIPE', v. %1.1f.\n", VNUM); 
                exit(EXIT_SUCCESS);
            case '?':
            default: 
                fprintf(stderr, "%s", header);
                fprintf(stderr, "%s", synops);
                exit(EXIT_FAILURE);
            argc -= optind; 
            argv += optind;
        }
    }
    // santiny-check the args
    if (min < 1.) { 
        fprintf(stderr, "Min pitch < 1 Hz, aborting.\n"); 
        exit(EXIT_FAILURE);
    }
    if (max - min < 1.) {
        fprintf(stderr, "Max pitch <= min pitch, aborting.\n"); 
        exit(EXIT_FAILURE);
    } 
    if (st < 0. || st > 1.) { 
        fprintf(stderr, "Strength must be 0 <= x <= 1, set to %.3f.\n", ST); 
        st = ST;
    }
    if (dt < .001) {
        fprintf(stderr, "Timestep must be >= 0.001, set to %.3f.\n", DT);
        dt = DT;
    }
    if (batch != NULL) { 
        // iterate through batch pairs
        while (fscanf(batch, "%s %s", wav, out) != EOF) {
            printf("%s -> %s...", wav, out);
            FILE* wf = fopen(wav, "r");
            if (wf == NULL) {
                fprintf(stderr, "Reading from \"%s\" failed.\n", wav);
                exit(EXIT_FAILURE);
            }
            vector p = swipe(fileno(wf), min, max, st, dt);
            fclose(wf);
            if (p.x == NOK) {
                fprintf(stderr, "Reading from \"%s\" failed.\n", wav);
                fclose(batch); 
                exit(EXIT_FAILURE);
            }
            else {
                FILE* output = fopen(out, "w");
                if (output == NULL) {
                    fprintf(stderr, "Writing to \"%s\" failed.\n", out);
                    exit(EXIT_FAILURE);
                }
                printp(p, fileno(output), dt, mel, vlo);
                printf("done.\n");
                fclose(output);
            }
            freev(p);
        }
        fclose(batch);
    }
    else {
        vector p;
        if (*wav == '\0') {
            p = swipe(fileno(stdin), min, max, st, dt);
            wav = "<STDIN>";
        }
        else {
            FILE* input = fopen(wav, "r");
            if (input == NULL) {
                fprintf(stderr, "Reading from \"%s\" failed (try ", wav);
                fprintf(stderr, "specifying an input file with -i).\n");
                exit(EXIT_FAILURE);
            }
            p = swipe(fileno(input), min, max, st, dt);
        }
        if (p.x == NOK) {
            if (*wav == '\0') {
                fprintf(stderr, "Reading from STDIN failed (did ");
                fprintf(stderr, "you pipe a file to `swipe`?').\n");
            }
            else {
                fprintf(stderr, "Reading from \"%s\" failed (try ", wav);
                fprintf(stderr, "specifying an input file with -i).\n");
            }
            exit(EXIT_FAILURE);
        }
        else {
            if (*out == '\0') 
                printp(p, fileno(stdout), dt, mel, vlo);
            else {
                FILE* output = fopen(out, "w");
                if (output == NULL) {
                    fprintf(stderr, "Writing to \"%s\" failed.\n", out);
                    exit(EXIT_FAILURE);
                }
                printp(p, fileno(output), dt, mel, vlo);
            }
        }
        freev(p);
    }
    exit(EXIT_SUCCESS);
}
#endif

#if 1
}  // namespace swipe
}  // namespace sptk
#endif
