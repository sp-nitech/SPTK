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

#include "SPTK/generation/recursive_maximum_likelihood_parameter_generation.h"

#include <algorithm>  // std::fill, std::max
#include <cfloat>     // DBL_MAX
#include <cstddef>    // std::size_t

namespace sptk {

RecursiveMaximumLikelihoodParameterGeneration::
    RecursiveMaximumLikelihoodParameterGeneration(
        int num_order, int num_past_frame,
        const std::vector<std::vector<double> >& window_coefficients,
        InputSourceInterface* input_source)
    : num_order_(num_order),
      num_past_frame_(num_past_frame),
      window_coefficients_(window_coefficients),
      input_source_(input_source),
      is_valid_(true) {
  if (num_order_ < 0 || num_past_frame_ < 0 || NULL == input_source_ ||
      !input_source_->IsValid()) {
    is_valid_ = false;
    return;
  }

  int max_half_window_width(0);
  for (std::vector<std::vector<double> >::iterator itr(
           window_coefficients_.begin());
       itr != window_coefficients_.end(); ++itr) {
    const int window_width(static_cast<int>(itr->size()));
    if (0 == window_width % 2) {
      itr->push_back(0.0);
    }
    const int half_window_width(window_width / 2);
    if (max_half_window_width < half_window_width) {
      max_half_window_width = half_window_width;
    }
  }

  if (num_past_frame_ < max_half_window_width) {
    is_valid_ = false;
    return;
  }

  // calculation_field is past + present + future.
  calculation_field_ = num_past_frame_ + 1 + max_half_window_width;
  num_remaining_frame_ = calculation_field_;
  current_frame_ = num_past_frame_;

  for (int i(1); i < calculation_field_; ++i) {
    if (!input_source->Get(&buffer_.static_and_dynamic_parameters)) {
      const int static_and_dynamic_size(
          static_cast<int>(buffer_.static_and_dynamic_parameters.size() / 2));
      // Initialize with zero mean.
      std::fill(buffer_.static_and_dynamic_parameters.begin(),
                buffer_.static_and_dynamic_parameters.begin() +
                    static_and_dynamic_size,
                0.0);
      // Initialize with infinite variance.
      std::fill(buffer_.static_and_dynamic_parameters.begin() +
                    static_and_dynamic_size,
                buffer_.static_and_dynamic_parameters.end(), DBL_MAX);
      --num_remaining_frame_;
    }
    if (!Forward()) {
      is_valid_ = false;
      return;
    }
  }
}

bool RecursiveMaximumLikelihoodParameterGeneration::Get(
    std::vector<double>* smoothed_static_parameters) {
  if (!is_valid_ || NULL == smoothed_static_parameters) {
    return false;
  }

  if (!input_source_->Get(&buffer_.static_and_dynamic_parameters)) {
    const int static_and_dynamic_size(
        static_cast<int>(buffer_.static_and_dynamic_parameters.size() / 2));
    // Unobserbed mean is zero.
    std::fill(
        buffer_.static_and_dynamic_parameters.begin(),
        buffer_.static_and_dynamic_parameters.begin() + static_and_dynamic_size,
        0.0);
    // Unobserved variance is infinite.
    std::fill(
        buffer_.static_and_dynamic_parameters.begin() + static_and_dynamic_size,
        buffer_.static_and_dynamic_parameters.end(), DBL_MAX);
    if (num_remaining_frame_ <= 1) {
      return false;
    }
    --num_remaining_frame_;
  }
  if (!Forward()) {
    return false;
  }

  const int static_size(GetSize());
  if (smoothed_static_parameters->size() !=
      static_cast<std::size_t>(static_size)) {
    smoothed_static_parameters->resize(static_size);
  }

  const int t((current_frame_ - num_past_frame_ - 1) % calculation_field_);
  double* output(&((*smoothed_static_parameters)[0]));
  for (int m(0); m < static_size; ++m) {
    output[m] = buffer_.c[m][t];
  }

  return true;
}

bool RecursiveMaximumLikelihoodParameterGeneration::Forward() {
  const int num_delta(static_cast<int>(window_coefficients_.size()));
  const int static_size(num_order_ + 1);
  const int dynamic_size(static_size * num_delta);

  // Prepare memories.
  {
    if (buffer_.stored_dynamic_mean_vectors.size() !=
        static_cast<std::size_t>(dynamic_size)) {
      buffer_.stored_dynamic_mean_vectors.resize(dynamic_size);
      for (int m(0); m < dynamic_size; ++m) {
        buffer_.stored_dynamic_mean_vectors[m].resize(calculation_field_);
      }
    }

    if (buffer_.stored_dynamic_diagonal_covariance_matrices.size() !=
        static_cast<std::size_t>(dynamic_size)) {
      buffer_.stored_dynamic_diagonal_covariance_matrices.resize(dynamic_size);
      for (int m(0); m < dynamic_size; ++m) {
        buffer_.stored_dynamic_diagonal_covariance_matrices[m].resize(
            calculation_field_);
      }
    }

    if (buffer_.pi.size() != static_cast<std::size_t>(static_size)) {
      buffer_.pi.resize(static_size);
      for (int m(0); m < static_size; ++m) {
        buffer_.pi[m].resize(calculation_field_);
      }
    }

    if (buffer_.k.size() != static_cast<std::size_t>(static_size)) {
      buffer_.k.resize(static_size);
      for (int m(0); m < static_size; ++m) {
        buffer_.k[m].resize(calculation_field_);
      }
    }

    if (buffer_.p.size() != static_cast<std::size_t>(static_size)) {
      buffer_.p.resize(static_size);
      for (int m(0); m < static_size; ++m) {
        buffer_.p[m].resize(calculation_field_);
        for (int u(0); u < calculation_field_; ++u) {
          buffer_.p[m][u].resize(2 * calculation_field_ + 1);
          std::fill(buffer_.p[m][u].begin(), buffer_.p[m][u].end(), 0.0);
          buffer_.p[m][u][calculation_field_] = DBL_MAX;
        }
      }
    }

    if (buffer_.c.size() != static_cast<std::size_t>(static_size)) {
      buffer_.c.resize(static_size);
      for (int m(0); m < static_size; ++m) {
        buffer_.c[m].resize(calculation_field_);
        std::fill(buffer_.c[m].begin(), buffer_.c[m].end(), 0.0);
      }
    }
  }

  // Copy inputs.
  const int max_half_window_width(calculation_field_ - num_past_frame_ - 1);
  {
    const int t((current_frame_ + max_half_window_width) % calculation_field_);

    const double* static_and_dynamic_mean_vector(
        &(buffer_.static_and_dynamic_parameters[0]));
    for (int m(0); m < static_size; ++m) {
      buffer_.c[m][t] = static_and_dynamic_mean_vector[m];
    }
    for (int m(0); m < dynamic_size; ++m) {
      buffer_.stored_dynamic_mean_vectors[m][t] =
          static_and_dynamic_mean_vector[static_size + m];
    }

    const double* static_and_dynamic_diagonal_covariance_matrix(
        &(buffer_.static_and_dynamic_parameters[static_size + dynamic_size]));
    for (int m(0); m < static_size; ++m) {
      std::fill(buffer_.p[m][t].begin(), buffer_.p[m][t].end(), 0.0);
    }
    for (int m(0); m < static_size; ++m) {
      buffer_.p[m][t][calculation_field_] =
          static_and_dynamic_diagonal_covariance_matrix[m];
    }
    for (int m(0); m < dynamic_size; ++m) {
      buffer_.stored_dynamic_diagonal_covariance_matrices[m][t] =
          static_and_dynamic_diagonal_covariance_matrix[static_size + m];
    }
  }

  for (int d(0); d < num_delta; ++d) {
    const int half_window_width(
        (static_cast<int>(window_coefficients_[d].size()) - 1) / 2);
    const double* window_coefficients(
        &(window_coefficients_[d][half_window_width]));

    // Do not update state if given variance is infinite.
    bool update(true);
    for (int m(0); m < static_size; ++m) {
      for (int j(-half_window_width); j <= half_window_width; ++j) {
        if (DBL_MAX == buffer_.p[m][(current_frame_ + j) % calculation_field_]
                                [calculation_field_]) {
          update = false;
          break;
        }
      }
      if (!update) break;
    }
    if (!update) continue;

    // Calculate the numerator of Kalman gain.
    for (int m(0); m < static_size; ++m) {
      double* pi(&buffer_.pi[m][num_past_frame_]);
      for (int u(-num_past_frame_); u <= max_half_window_width; ++u) {
        double tmp(0.0);
        for (int j(-half_window_width); j <= half_window_width; ++j) {
          double* p(&buffer_.p[m][(current_frame_ + j) % calculation_field_]
                              [calculation_field_]);
          tmp += window_coefficients[j] * p[u - j];
        }
        pi[u] = tmp;
      }
    }

    // Calculate Kalman gain.
    for (int m(0); m < static_size; ++m) {
      double* pi(&buffer_.pi[m][num_past_frame_]);
      double tmp(0.0);
      for (int j(-half_window_width); j <= half_window_width; ++j) {
        tmp += window_coefficients[j] * pi[j];
      }

      const double denominator(
          1.0 /
          (tmp +
           buffer_.stored_dynamic_diagonal_covariance_matrices
               [static_size * d + m][current_frame_ % calculation_field_]));
      double* k(&buffer_.k[m][num_past_frame_]);
      for (int u(-num_past_frame_); u <= max_half_window_width; ++u) {
        k[u] = pi[u] * denominator;
      }
    }

    // Update error covariance.
    for (int m(0); m < static_size; ++m) {
      double* pi(&buffer_.pi[m][num_past_frame_]);
      double* k(&buffer_.k[m][num_past_frame_]);
      for (int u(-num_past_frame_); u <= max_half_window_width; ++u) {
        double* pu(&buffer_.p[m][(current_frame_ + u) % calculation_field_]
                             [calculation_field_]);
        for (int v(std::max(u, -half_window_width)); v <= max_half_window_width;
             ++v) {
          pu[v - u] -= k[v] * pi[u];
          if (v != u) {
            double* pv(&buffer_.p[m][(current_frame_ + v) % calculation_field_]
                                 [calculation_field_]);
            pv[u - v] = pu[v - u];
          }
        }
      }
    }

    // Update state estimates.
    for (int m(0); m < static_size; ++m) {
      double* c(&buffer_.c[m][0]);
      double tmp(buffer_.stored_dynamic_mean_vectors[static_size * d + m]
                                                    [current_frame_ %
                                                     calculation_field_]);
      for (int j(-half_window_width); j <= half_window_width; ++j) {
        tmp -= window_coefficients[j] *
               c[(current_frame_ + j) % calculation_field_];
      }

      double* k(&buffer_.k[m][num_past_frame_]);
      for (int u(-num_past_frame_); u <= max_half_window_width; ++u) {
        c[(current_frame_ + u) % calculation_field_] += k[u] * tmp;
      }
    }
  }

  ++current_frame_;

  return true;
}

}  // namespace sptk
