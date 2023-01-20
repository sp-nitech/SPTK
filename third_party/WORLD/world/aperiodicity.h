//-----------------------------------------------------------------------------
// Copyright 2012-2015 Masanori Morise. All Rights Reserved.
// Author: mmorise [at] yamanashi.ac.jp (Masanori Morise)
//-----------------------------------------------------------------------------
#ifndef WORLD_APERIODICITY_H_
#define WORLD_APERIODICITY_H_

#if 1
namespace sptk {
namespace world {
#endif

//-----------------------------------------------------------------------------
// The latest version of aperiodicity estimation in TANDEM-STRAIGHT.
// This function skipped several complex processes.
// Input:
//   x            : Input signal
//   x_length     : Length of x
//   f0           : f0 contour
//   f0_length    : Length of f0
//   frame_period : Time interval for analysis
// Output:
//   aperiodicity : Estimated aperiodicity
//   Value used for the aperiodicity estimation. This value is used for
//   the synthesis.
//-----------------------------------------------------------------------------
void AperiodicityRatio(double *x, int x_length, int fs, double *f0,
    int f0_length, double *time_axis, int fft_size, double **aperiodicity);

#if 1
}  // namespace world
}  // namespace sptk
#endif

#endif  // WORLD_APERIODICITY_H_
