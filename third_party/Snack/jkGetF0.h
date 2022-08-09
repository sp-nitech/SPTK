/*
 * This software has been licensed to the Centre of Speech Technology, KTH
 * by Microsoft Corp. with the terms in the accompanying file BSD.txt,
 * which is a BSD style license.
 *
 *    "Copyright (c) 1990-1996 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:
 * @(#)f0.h	1.4 9/9/96 ERL
 * Brief description:
 *
 */

/* #if 1 */
#ifndef JK_GET_F0_H
#define JK_GET_F0_H
/* #endif */

#if 1
#include <stdlib.h>
#include <vector>

namespace sptk {
namespace snack {
#endif

/* f0.h */
/* Some definitions used by the "Pitch Tracker Software". */
       
typedef struct f0_params {
float cand_thresh,	/* only correlation peaks above this are considered */
      lag_weight,	/* degree to which shorter lags are weighted */
      freq_weight,	/* weighting given to F0 trajectory smoothness */
      trans_cost,	/* fixed cost for a voicing-state transition */
      trans_amp,	/* amplitude-change-modulated VUV trans. cost */
      trans_spec,	/* spectral-change-modulated VUV trans. cost */
      voice_bias,	/* fixed bias towards the voiced hypothesis */
      double_cost,	/* cost for octave F0 jumps */
      mean_f0,		/* talker-specific mean F0 (Hz) */
      mean_f0_weight,	/* weight to be given to deviations from mean F0 */
      min_f0,		/* min. F0 to search for (Hz) */
      max_f0,		/* max. F0 to search for (Hz) */
      frame_step,	/* inter-frame-interval (sec) */
      wind_dur;		/* duration of correlation window (sec) */
int   n_cands,		/* max. # of F0 cands. to consider at each frame */
      conditioning;     /* Specify optional signal pre-conditioning. */
} F0_params;

#if 0
/* Possible values returned by the function f0(). */
#define F0_OK		0
#define F0_NO_RETURNS	1
#define F0_TOO_FEW_SAMPLES	2
#define F0_NO_INPUT	3
#define F0_NO_PAR	4
#define F0_BAD_PAR	5
#define F0_BAD_INPUT	6
#define F0_INTERNAL_ERR	7

/* Bits to specify optional pre-conditioning of speech signals by f0() */
/* These may be OR'ed together to specify all preprocessing. */
#define F0_PC_NONE	0x00		/* no pre-processing */
#define F0_PC_DC	0x01		/* remove DC */
#define F0_PC_LP2000	0x02		/* 2000 Hz lowpass */
#define F0_PC_HP100	0x04		/* 100 Hz highpass */
#define F0_PC_AR	0x08		/* inf_order-order LPC inverse filter */
#define F0_PC_DIFF	0x010		/* 1st-order difference */

extern F0_params *new_f0_params();
extern int atoi(), eround(), lpc(), window(), get_window();
extern void get_fast_cands(), a_to_aca(), cross(), crossf(), crossfi(),
           autoc(), durbin();
#else
int eround(double flnum);
int window(float *din, float *dout, int n, float preemp, int type,
           int *wsizep, float **windp);
void crossf(float *data, int size, int start, int nlags, float *engref,
            int *maxloc, float *maxval, float *correl, int *dbsizep,
            float **dbdatap);
void crossfi(float *data, int size, int start0, int nlags0, int nlags,
             float *engref, int *maxloc, float *maxval, float *correl,
             int *locs, int nlocs, int *dbsizep, float **dbdatap);
#endif

#define Fprintf (void)fprintf

/* f0_structs.h */

#define BIGSORD 100

typedef struct cross_rec { /* for storing the crosscorrelation information */
	float	rms;	/* rms energy in the reference window */
	float	maxval;	/* max in the crosscorr. fun. q15 */
	short	maxloc; /* lag # at which max occured	*/
	short	firstlag; /* the first non-zero lag computed */
	float	*correl; /* the normalized corsscor. fun. q15 */
} Cross;

typedef struct dp_rec { /* for storing the DP information */
	short	ncands;	/* # of candidate pitch intervals in the frame */
	short	*locs; /* locations of the candidates */
	float	*pvals; /* peak values of the candidates */
	float	*mpvals; /* modified peak values of the candidates */
	short	*prept; /* pointers to best previous cands. */
	float	*dpvals; /* cumulative error for each candidate */
} Dprec;

typedef struct windstat_rec {  /* for lpc stat measure in a window */
    float rho[BIGSORD+1];
    float err;
    float rms;
} Windstat;

typedef struct sta_rec {  /* for stationarity measure */
  float *stat;
  float *rms;
  float *rms_ratio;
} Stat;


typedef struct frame_rec{
  Cross *cp;
  Dprec *dp;
  float rms;
  struct frame_rec *next;
  struct frame_rec *prev;
} Frame;

#if 0
extern   Frame *alloc_frame();
#else
Frame *alloc_frame(int nlags, int ncands);
#endif

#if 1
void *ckalloc(size_t size);
void *ckrealloc(void *ptr, size_t size);
void ckfree(void* ptr);
int cGet_f0(const std::vector<double> &waveform, int frame_shift,
            double sample_freq, double min_f0, double max_f0, double voice_bias,
            float **outlist, int *length);
#endif

#if 1
}  /* namespace snack */
}  /* namespace sptk */
#endif

/* #if 1 */
#endif
/* #endif */
