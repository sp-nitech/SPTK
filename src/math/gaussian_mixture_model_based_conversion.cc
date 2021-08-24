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

#include "SPTK/math/gaussian_mixture_model_based_conversion.h"

#include <algorithm>  // std::copy, std::max_element
#include <cstddef>    // std::size_t

#include "SPTK/math/gaussian_mixture_modeling.h"

namespace sptk {

GaussianMixtureModelBasedConversion::GaussianMixtureModelBasedConversion(
    int num_source_order, int num_target_order,
    const std::vector<std::vector<double> >& window_coefficients,
    const std::vector<double>& weights,
    const std::vector<std::vector<double> >& mean_vectors,
    const std::vector<SymmetricMatrix>& covariance_matrices,
    bool use_magic_number, double magic_number)
    : num_source_order_(num_source_order),
      num_target_order_(num_target_order),
      source_length_((num_source_order_ + 1) *
                     (static_cast<int>(window_coefficients.size() + 1))),
      target_length_((num_target_order_ + 1) *
                     (static_cast<int>(window_coefficients.size() + 1))),
      weights_(weights),
      use_magic_number_(use_magic_number),
      magic_number_(magic_number),
      num_mixture_(static_cast<int>(weights_.size())),
      mlpg_(num_target_order_, window_coefficients, use_magic_number_,
            magic_number_),
      is_valid_(true),
      source_mean_vectors_(num_mixture_, std::vector<double>(source_length_)),
      source_covariance_matrices_(num_mixture_,
                                  SymmetricMatrix(source_length_)),
      e_slope_(num_mixture_, Matrix(target_length_, source_length_)),
      e_bias_(num_mixture_, std::vector<double>(target_length_)),
      d_(num_mixture_, SymmetricMatrix(target_length_)) {
  if (num_source_order_ < 0 || num_target_order_ < 0 || num_mixture_ <= 0 ||
      !mlpg_.IsValid()) {
    is_valid_ = false;
    return;
  }

  for (int k(0); k < num_mixture_; ++k) {
    // Check size of mean vectors and covariance matrices.
    const int total_length(source_length_ + target_length_);
    if (mean_vectors[k].size() != static_cast<std::size_t>(total_length) ||
        covariance_matrices[k].GetNumDimension() != total_length) {
      is_valid_ = false;
      return;
    }

    // Set \mu^{(X)}.
    std::copy(mean_vectors[k].begin(), mean_vectors[k].begin() + source_length_,
              source_mean_vectors_[k].begin());

    // Set \Sigma^{(XX)}.
    for (int l(0); l < source_length_; ++l) {
      for (int m(0); m <= l; ++m) {
        source_covariance_matrices_[k][l][m] = covariance_matrices[k][l][m];
      }
    }

    // Set \Sigma^{(YX)} \Sigma^{(XX)}^{-1}.
    SymmetricMatrix xx(source_length_);
    if (!source_covariance_matrices_[k].Invert(&xx)) {
      is_valid_ = false;
      return;
    }
    for (int l(0); l < target_length_; ++l) {
      const int ll(source_length_ + l);
      for (int m(0); m < source_length_; ++m) {
        for (int n(0); n < source_length_; ++n) {
          e_slope_[k][l][m] += covariance_matrices[k][ll][n] * xx[n][m];
        }
      }
    }

    // Set \mu^{(Y)} - \Sigma^{(YX)} \Sigma^{(XX)}^{-1} \mu^{(X)}
    for (int l(0); l < target_length_; ++l) {
      const int ll(l + source_length_);
      double tmp(0.0);
      for (int m(0); m < source_length_; ++m) {
        tmp += e_slope_[k][l][m] * mean_vectors[k][m];
      }
      e_bias_[k][l] = mean_vectors[k][ll] - tmp;
    }

    // Set D^{(Y)}.
    for (int l(0); l < target_length_; ++l) {
      const int ll(source_length_ + l);
      for (int m(0); m <= l; ++m) {
        const int mm(source_length_ + m);
        double tmp(0.0);
        for (int n(0); n < source_length_; ++n) {
          tmp += e_slope_[k][l][n] * covariance_matrices[k][n][mm];
        }
        d_[k][l][m] = covariance_matrices[k][ll][mm] - tmp;
      }
    }
  }
}

bool GaussianMixtureModelBasedConversion::Run(
    const std::vector<std::vector<double> >& source_vectors,
    std::vector<std::vector<double> >* target_vectors) const {
  // Check inputs.
  if (!is_valid_ || source_vectors.empty() || NULL == target_vectors) {
    return false;
  }

  // Prepare memories.
  const int sequence_length(static_cast<int>(source_vectors.size()));
  std::vector<std::vector<double> > e(
      sequence_length, std::vector<double>(target_length_, magic_number_));
  std::vector<SymmetricMatrix> d(sequence_length,
                                 SymmetricMatrix(target_length_));

  // Compute posterior probabilities of source input.
  {
    std::vector<double> components_of_log_probability(num_mixture_);
    GaussianMixtureModeling::Buffer buffer;
    for (int t(0); t < sequence_length; ++t) {
      if (use_magic_number_ && magic_number_ == source_vectors[t][0]) {
        continue;
      }

      if (!GaussianMixtureModeling::CalculateLogProbability(
              source_length_ - 1, num_mixture_, false, 0 == t,
              source_vectors[t], weights_, source_mean_vectors_,
              source_covariance_matrices_, &components_of_log_probability, NULL,
              &buffer)) {
        return false;
      }

      const int selected_mixture(static_cast<int>(
          std::max_element(components_of_log_probability.begin(),
                           components_of_log_probability.end()) -
          components_of_log_probability.begin()));

      // Set E.
      for (int l(0); l < target_length_; ++l) {
        double tmp(0.0);
        for (int m(0); m < source_length_; ++m) {
          tmp += e_slope_[selected_mixture][l][m] * source_vectors[t][m];
        }
        e[t][l] = e_bias_[selected_mixture][l] + tmp;
      }

      // Set D.
      d[t] = d_[selected_mixture];
    }
  }

  if (!mlpg_.Run(e, d, target_vectors)) {
    return false;
  }

  return true;
}

}  // namespace sptk
