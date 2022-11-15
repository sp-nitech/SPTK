// ------------------------------------------------------------------------ //
// Copyright 2021 SPTK Working Group                                        //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ------------------------------------------------------------------------ //

#include "SPTK/compression/dynamic_range_compression.h"

#include <algorithm>  // std::max
#include <cmath>      // std::fabs, std::exp, std::pow

namespace {

double WeightedSum(double a, double x, double y) {
  return a * x + (1.0 - a) * y;
}

}  // namespace

namespace sptk {

DynamicRangeCompression::DynamicRangeCompression(
    double abs_max_value, double sampling_rate, double threshold, double ratio,
    double knee_width, double attack_time, double release_time,
    double makeup_gain, DetectorType detector_type)
    : abs_max_value_(abs_max_value),
      sampling_rate_(sampling_rate),
      threshold_(threshold),
      ratio_(ratio),
      knee_width_(knee_width),
      attack_time_(attack_time),
      release_time_(release_time),
      makeup_gain_(makeup_gain),
      detector_type_(detector_type),
      alpha_a_(std::exp(-1000.0 / (sampling_rate_ * attack_time_))),
      alpha_r_(std::exp(-1000.0 / (sampling_rate_ * release_time_))),
      is_valid_(true) {
  if (abs_max_value <= 0.0 || sampling_rate_ <= 0.0 || 0.0 < threshold_ ||
      ratio_ < 1.0 || knee_width_ < 0.0 || attack_time_ <= 0.0 ||
      release_time_ <= 0.0 || makeup_gain_ < 0.0) {
    is_valid_ = false;
    return;
  }
}

bool DynamicRangeCompression::Run(
    const double input, double* output,
    DynamicRangeCompression::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || NULL == output || NULL == buffer) {
    return false;
  }

  // Convert to decibels.
  const double xg(20.0 * sptk::FloorLog10(std::fabs(input / abs_max_value_)));

  // Gain computer.
  double yg;
  {
    const double t1(threshold_ - 0.5 * knee_width_);
    const double t2(t1 + knee_width_);
    if (xg <= t1) {
      yg = xg;
    } else if (t2 <= xg) {
      yg = threshold_ + (xg - threshold_) / ratio_;
    } else {
      const double x(xg - t1);
      yg = xg + (1.0 / ratio_ - 1) * x * x / (2.0 * knee_width_);
    }
  }

  // Level detector.
  double yl;
  {
    const double xl(xg - yg);
    switch (detector_type_) {
      case kBranching: {
        if (buffer->prev_yl < xl) {
          yl = WeightedSum(alpha_a_, buffer->prev_yl, xl);
        } else {
          yl = alpha_r_ * buffer->prev_yl;
        }
        break;
      }
      case kDecoupled: {
        const double y1(std::max(xl, alpha_r_ * buffer->prev_y1));
        yl = WeightedSum(alpha_a_, buffer->prev_yl, y1);
        buffer->prev_y1 = y1;
        break;
      }
      case kBranchingSmooth: {
        if (buffer->prev_yl < xl) {
          yl = WeightedSum(alpha_a_, buffer->prev_yl, xl);
        } else {
          yl = WeightedSum(alpha_r_, buffer->prev_yl, xl);
        }
        break;
      }
      case kDecoupledSmooth: {
        const double y1(
            std::max(xl, WeightedSum(alpha_r_, buffer->prev_y1, xl)));
        yl = WeightedSum(alpha_a_, buffer->prev_yl, y1);
        buffer->prev_y1 = y1;
        break;
      }
      default: {
        return false;
      }
    }
    buffer->prev_yl = yl;
  }

  // Compress input.
  const double g(std::pow(10.0, 0.05 * (makeup_gain_ - yl)));
  *output = input * g;

  return true;
}

bool DynamicRangeCompression::Run(
    double* input_and_output, DynamicRangeCompression::Buffer* buffer) const {
  if (NULL == input_and_output) return false;
  return Run(*input_and_output, input_and_output, buffer);
}

}  // namespace sptk
