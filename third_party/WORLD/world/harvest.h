//-----------------------------------------------------------------------------
// Copyright 2012 Masanori Morise
// Author: mmorise [at] meiji.ac.jp (Masanori Morise)
// Last update: 2021/02/15
//-----------------------------------------------------------------------------
#ifndef WORLD_HARVEST_H_
#define WORLD_HARVEST_H_

#if 0
#include "world/macrodefinitions.h"

WORLD_BEGIN_C_DECLS
#else
namespace sptk {
namespace world {
#endif

//-----------------------------------------------------------------------------
// Struct for Harvest
//-----------------------------------------------------------------------------
typedef struct {
  double f0_floor;
  double f0_ceil;
  double frame_period;
#if 1
  double allowed_range;
#endif
} HarvestOption;

//-----------------------------------------------------------------------------
// Harvest
//
// Input:
//   x                    : Input signal
//   x_length             : Length of x
//   fs                   : Sampling frequency
//   option               : Struct to order the parameter for Harvest
//
// Output:
//   temporal_positions   : Temporal positions.
//   f0                   : F0 contour.
//-----------------------------------------------------------------------------
void Harvest(const double *x, int x_length, int fs,
  const HarvestOption *option, double *temporal_positions, double *f0);

//-----------------------------------------------------------------------------
// InitializeHarvestOption allocates the memory to the struct and sets the
// default parameters.
//
// Output:
//   option   : Struct for the optional parameter.
//-----------------------------------------------------------------------------
void InitializeHarvestOption(HarvestOption *option);

//-----------------------------------------------------------------------------
// GetSamplesForHarvest() calculates the number of samples required for
// Harvest().
//
// Input:
//   fs             : Sampling frequency [Hz]
//   x_length       : Length of the input signal [Sample]
//   frame_period   : Frame shift [msec]
//
// Output:
//   The number of samples required to store the results of Harvest().
//-----------------------------------------------------------------------------
int GetSamplesForHarvest(int fs, int x_length, double frame_period);

#if 0
WORLD_END_C_DECLS
#else
}  // namespace world
}  // namespace sptk
#endif

#endif  // WORLD_HARVEST_H_
