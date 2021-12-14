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

#include "SPTK/math/gaussian_mixture_modeling.h"

#include <algorithm>  // std::fill, std::transform
#include <cfloat>     // DBL_MAX
#include <cmath>      // std::exp, std::log
#include <cstddef>    // std::size_t
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::endl
#include <numeric>    // std::accumulate, std::partial_sum

#include "SPTK/compression/linde_buzo_gray_algorithm.h"
#include "SPTK/math/statistics_accumulation.h"

namespace {

bool CheckGmm(int num_mixture, int length, const std::vector<double>& weights,
              const std::vector<std::vector<double> >& mean_vectors,
              const std::vector<sptk::SymmetricMatrix>& covariance_matrices) {
  if (weights.size() != static_cast<std::size_t>(num_mixture)) {
    return false;
  }
  if (mean_vectors.size() != static_cast<std::size_t>(num_mixture)) {
    return false;
  }
  for (int k(0); k < num_mixture; ++k) {
    if (mean_vectors[k].size() != static_cast<std::size_t>(length)) {
      return false;
    }
  }
  if (covariance_matrices.size() != static_cast<std::size_t>(num_mixture)) {
    return false;
  }
  for (int k(0); k < num_mixture; ++k) {
    if (covariance_matrices[k].GetNumDimension() != length) {
      return false;
    }
  }
  return true;
}

}  // namespace

namespace sptk {

GaussianMixtureModeling::GaussianMixtureModeling(
    int num_order, int num_mixture, int num_iteration,
    double convergence_threshold, CovarianceType covariance_type,
    std::vector<int> block_size, double weight_floor, double variance_floor,
    InitializationType initialization_type, int log_interval,
    double smoothing_parameter, const std::vector<double>& ubm_weights,
    const std::vector<std::vector<double> >& ubm_mean_vectors,
    const std::vector<SymmetricMatrix>& ubm_covariance_matrices)
    : num_order_(num_order),
      num_mixture_(num_mixture),
      num_iteration_(num_iteration),
      convergence_threshold_(convergence_threshold),
      covariance_type_(covariance_type),
      block_size_(block_size),
      weight_floor_(weight_floor),
      variance_floor_(variance_floor),
      initialization_type_(initialization_type),
      log_interval_(log_interval),
      smoothing_parameter_(smoothing_parameter),
      ubm_weights_(ubm_weights),
      ubm_mean_vectors_(ubm_mean_vectors),
      ubm_covariance_matrices_(ubm_covariance_matrices),
      is_diagonal_(kDiagonal == covariance_type_ && 1 == block_size_.size()),
      is_valid_(true) {
  if (num_order_ < 0 || num_mixture_ <= 0 ||
      (kKMeans == initialization_type_ && !IsPowerOfTwo(num_mixture_)) ||
      num_iteration_ <= 0 || convergence_threshold_ < 0.0 ||
      weight_floor_ < 0.0 || 1.0 / num_mixture_ < weight_floor_ ||
      variance_floor_ < 0.0 || log_interval <= 0 ||
      smoothing_parameter_ < 0.0 || 1.0 < smoothing_parameter_) {
    is_valid_ = false;
    return;
  }

  const int length(num_order_ + 1);

  // Check block size.
  {
    int sum(0);
    for (int b : block_size) {
      if (b <= 0) {
        is_valid_ = false;
        return;
      }
      sum += b;
    }
    if (sum != length) {
      is_valid_ = false;
      return;
    }
  }

  // Create mask of covariance.
  {
    const int num_block(static_cast<int>(block_size_.size()));
    std::vector<int> offset(num_block + 1);
    std::partial_sum(block_size_.begin(), block_size_.end(),
                     offset.begin() + 1);

    mask_.Resize(length);
    mask_.Fill(1.0);
    for (int row(0); row < num_block; ++row) {
      const int begin_l(offset[row]);
      const int end_l(offset[row] + block_size_[row]);
      for (int col(0); col < num_block; ++col) {
        const int begin_m(offset[col]);
        const int end_m(offset[col] + block_size_[col]);
        for (int l(begin_l); l < end_l; ++l) {
          for (int m(begin_m); m < end_m; ++m) {
            if (kDiagonal == covariance_type_) {
              if (block_size_[row] != block_size_[col]) {
                mask_[l][m] = 0.0;
              } else if (offset[row + 1] - l != offset[col + 1] - m) {
                mask_[l][m] = 0.0;
              }
            } else if (kFull == covariance_type_) {
              if (row != col) {
                mask_[l][m] = 0.0;
              }
            }
          }
        }
      }
    }
  }

  // Check the sanity of universal background model.
  if (kUbm == initialization_type_ || 0.0 < smoothing_parameter_) {
    if (!CheckGmm(num_mixture, length, ubm_weights, ubm_mean_vectors,
                  ubm_covariance_matrices)) {
      is_valid_ = false;
      return;
    }
  }

  if (0.0 < smoothing_parameter_) {
    xi_.resize(num_mixture_);
    std::transform(
        ubm_weights.begin(), ubm_weights.end(), xi_.begin(),
        [smoothing_parameter](double w) { return smoothing_parameter * w; });
  }
}

bool GaussianMixtureModeling::Run(
    const std::vector<std::vector<double> >& input_vectors,
    std::vector<double>* weights,
    std::vector<std::vector<double> >* mean_vectors,
    std::vector<SymmetricMatrix>* covariance_matrices) const {
  // Check inputs.
  if (!is_valid_ || input_vectors.empty() || NULL == weights ||
      NULL == mean_vectors || NULL == covariance_matrices) {
    return false;
  }

  // Initialize GMM parameters.
  const int length(num_order_ + 1);
  switch (initialization_type_) {
    case kNone: {
      if (!CheckGmm(num_mixture_, length, *weights, *mean_vectors,
                    *covariance_matrices)) {
        return false;
      }
      break;
    }
    case kKMeans: {
      if (!Initialize(input_vectors, weights, mean_vectors,
                      covariance_matrices)) {
        return false;
      }
      break;
    }
    case kUbm: {
      *weights = ubm_weights_;
      *mean_vectors = ubm_mean_vectors_;
      *covariance_matrices = ubm_covariance_matrices_;
      break;
    }
    default: {
      return false;
    }
  }

  // Prepare memories.
  std::vector<double> buffer0(num_mixture_);
  std::vector<std::vector<double> > buffer1(num_mixture_);
  for (int k(0); k < num_mixture_; ++k) {
    buffer1[k].resize(length);
  }
  std::vector<SymmetricMatrix> buffer2(num_mixture_);
  for (int k(0); k < num_mixture_; ++k) {
    buffer2[k].Resize(length);
  }
  GaussianMixtureModeling::Buffer buffer;
  std::vector<double> numerators(num_mixture_);

  double prev_log_likelihood(-DBL_MAX);

  for (int n(1); n <= num_iteration_; ++n) {
    // Clear buffers.
    std::fill(buffer0.begin(), buffer0.end(), 0.0);
    for (int k(0); k < num_mixture_; ++k) {
      std::fill(buffer1[k].begin(), buffer1[k].end(), 0.0);
      buffer2[k].Fill(0.0);
    }
    buffer.precomputed_ = false;

    // Perform E-step.
    double log_likelihood(0.0);
    const int num_data(static_cast<int>(input_vectors.size()));
    for (int t(0); t < num_data; ++t) {
      // Compute log-likelihood of data.
      double denominator;
      if (!CalculateLogProbability(num_order_, num_mixture_, is_diagonal_,
                                   false, input_vectors[t], *weights,
                                   *mean_vectors, *covariance_matrices,
                                   &numerators, &denominator, &buffer)) {
        return false;
      }
      log_likelihood += denominator;

      const double* x(&(input_vectors[t][0]));
      for (int k(0); k < num_mixture_; ++k) {
        const double posterior(std::exp(numerators[k] - denominator));

        // Accumulate zeroth-order statistics.
        buffer0[k] += posterior;

        // Accumulate first-order statistics.
        for (int l(0); l <= num_order_; ++l) {
          buffer1[k][l] += posterior * x[l];
        }

        // Accumulate second-order statistics.
        for (int l(0); l <= num_order_; ++l) {
          for (int m(is_diagonal_ ? l : 0); m <= l; ++m) {
            if (0.0 != mask_[l][m]) {
              buffer2[k][l][m] += posterior * x[l] * x[m];
            }
          }
        }
      }
    }

    // Update mixture weights.
    if (0.0 == smoothing_parameter_) {
      const double z(1.0 / num_data);
      for (int k(0); k < num_mixture_; ++k) {
        (*weights)[k] = buffer0[k] * z;
      }
    } else {
      const double z(1.0 /
                     (num_data + std::accumulate(xi_.begin(), xi_.end(), 0.0)));
      for (int k(0); k < num_mixture_; ++k) {
        (*weights)[k] = (buffer0[k] + xi_[k]) * z;
      }
    }
    FloorWeight(weights);

    // Update mean vectors.
    if (0.0 == smoothing_parameter_) {
      for (int k(0); k < num_mixture_; ++k) {
        double* mu(&((*mean_vectors)[k][0]));
        double z(1.0 / buffer0[k]);
        for (int l(0); l <= num_order_; ++l) {
          mu[l] = buffer1[k][l] * z;
        }
      }
    } else {
      for (int k(0); k < num_mixture_; ++k) {
        double* mu(&((*mean_vectors)[k][0]));
        double z(1.0 / (buffer0[k] + xi_[k]));
        for (int l(0); l <= num_order_; ++l) {
          mu[l] = (buffer1[k][l] + xi_[k] * ubm_mean_vectors_[k][l]) * z;
        }
      }
    }

    // Update covariance matrices.
    if (0.0 == smoothing_parameter_) {
      for (int k(0); k < num_mixture_; ++k) {
        double* mu(&((*mean_vectors)[k][0]));
        double z(1.0 / buffer0[k]);
        for (int l(0); l <= num_order_; ++l) {
          for (int m(is_diagonal_ ? l : 0); m <= l; ++m) {
            if (0.0 != mask_[l][m]) {
              (*covariance_matrices)[k][l][m] =
                  buffer2[k][l][m] * z - (mu[l] * mu[m]);
            }
          }
        }
      }
    } else {
      for (int k(0); k < num_mixture_; ++k) {
        double* mu(&((*mean_vectors)[k][0]));
        double z(1.0 / (buffer0[k] + xi_[k]));
        for (int l(0); l <= num_order_; ++l) {
          for (int m(is_diagonal_ ? l : 0); m <= l; ++m) {
            if (0.0 != mask_[l][m]) {
              const double mu_l(buffer1[k][l] / buffer0[k]);
              const double mu_m(buffer1[k][m] / buffer0[k]);
              const double a(buffer2[k][l][m] -
                             buffer0[k] *
                                 (mu_l * mu[m] + mu[l] * mu_m - mu[l] * mu[m]));
              const double b(xi_[k] * ubm_covariance_matrices_[k][l][m]);
              const double c(xi_[k] * (ubm_mean_vectors_[k][l] - mu[l]) *
                             (ubm_mean_vectors_[k][m] - mu[m]));
              (*covariance_matrices)[k][l][m] = (a + b + c) * z;
            }
          }
        }
      }
    }
    FloorVariance(covariance_matrices);

    // Check convergence.
    log_likelihood /= num_data;
    const double change(log_likelihood - prev_log_likelihood);
    if (0 == n % log_interval_) {
      std::cerr << "iter " << std::setw(3) << n << " : ";
      std::cerr << "average = " << log_likelihood;
      if (1 == n) {
        std::cerr << std::endl;
      } else {
        std::cerr << ", change = " << change << std::endl;
      }
    }
    if (change < convergence_threshold_) {
      break;
    }
    prev_log_likelihood = log_likelihood;
  }

  return true;
}

bool GaussianMixtureModeling::CalculateLogProbability(
    int num_order, int num_mixture, bool is_diagonal, bool check_size,
    const std::vector<double>& input_vector, const std::vector<double>& weights,
    const std::vector<std::vector<double> >& mean_vectors,
    const std::vector<SymmetricMatrix>& covariance_matrices,
    std::vector<double>* components_of_log_probability, double* log_probability,
    GaussianMixtureModeling::Buffer* buffer) {
  // Check inputs.
  const int length(num_order + 1);
  if (num_mixture < 0 ||
      input_vector.size() != static_cast<std::size_t>(length) ||
      NULL == buffer) {
    return false;
  }

  // Check size of GMM.
  if (check_size && !CheckGmm(num_mixture, length, weights, mean_vectors,
                              covariance_matrices)) {
    return false;
  }

  // Prepare memories.
  if (components_of_log_probability &&
      components_of_log_probability->size() !=
          static_cast<std::size_t>(num_mixture)) {
    components_of_log_probability->resize(num_mixture);
  }
  if (buffer->d_.size() != static_cast<std::size_t>(length)) {
    buffer->d_.resize(length);
  }
  if (buffer->gconsts_.size() != static_cast<std::size_t>(num_mixture)) {
    buffer->gconsts_.resize(num_mixture);
  }
  if (buffer->precisions_.size() != static_cast<std::size_t>(num_mixture)) {
    buffer->precisions_.resize(num_mixture);
  }

  if (!buffer->precomputed_) {
    // Precompute constant of log likelihood without multiplying -0.5.
    for (int k(0); k < num_mixture; ++k) {
      double gconst(length * std::log(sptk::kTwoPi));
      double log_determinant(0.0);
      if (is_diagonal) {
        for (int l(0); l <= num_order; ++l) {
          log_determinant += std::log(covariance_matrices[k][l][l]);
        }
        gconst += log_determinant;
      } else {
        SymmetricMatrix tmp;
        std::vector<double> diag;
        if (!covariance_matrices[k].CholeskyDecomposition(&tmp, &diag)) {
          return false;
        }
        log_determinant += std::accumulate(
            diag.begin(), diag.end(), 0.0,
            [](double acc, double x) { return acc + std::log(x); });
        gconst += log_determinant;
      }
      buffer->gconsts_[k] = gconst;
    }

    // Precompute inverse of covariance matrix.
    if (!is_diagonal) {
      for (int k(0); k < num_mixture; ++k) {
        if (!covariance_matrices[k].Invert(&(buffer->precisions_[k]))) {
          return false;
        }
      }
    }

    buffer->precomputed_ = true;
  }

  const double* x(&(input_vector[0]));
  double total(sptk::kLogZero);

  // Compute log probability of data.
  for (int k(0); k < num_mixture; ++k) {
    double sum(buffer->gconsts_[k]);
    const double* mu(&(mean_vectors[k][0]));
    if (is_diagonal) {
      for (int l(0); l <= num_order; ++l) {
        const double diff(x[l] - mu[l]);
        sum += diff * diff / covariance_matrices[k][l][l];
      }
    } else {
      double* d(&(buffer->d_[0]));
      for (int l(0); l <= num_order; ++l) {
        d[l] = x[l] - mu[l];
      }
      for (int l(0); l <= num_order; ++l) {
        double tmp(0.0);
        for (int m(0); m <= num_order; ++m) {
          tmp += d[m] * buffer->precisions_[k][l][m];
        }
        sum += tmp * d[l];
      }
    }

    const double p(std::log(weights[k]) - 0.5 * sum);
    if (components_of_log_probability) {
      (*components_of_log_probability)[k] = p;
    }
    total = AddInLogSpace(total, p);
  }

  if (log_probability) {
    *log_probability = total;
  }

  return true;
}

void GaussianMixtureModeling::FloorWeight(std::vector<double>* weights) const {
  double sum(0.0);
  double* w(&((*weights)[0]));
  for (int k(0); k < num_mixture_; ++k) {
    if (w[k] < weight_floor_) {
      w[k] = weight_floor_;
    }
    sum += w[k];
  }

  // Re-normalize weights.
  if (1.0 != sum) {
    const double sum_floor(weight_floor_ * num_mixture_);
    const double a((1.0 - sum_floor) / (sum - sum_floor));
    const double b(weight_floor_ * (1.0 - a));
    for (int k(0); k < num_mixture_; ++k) {
      w[k] = a * w[k] + b;
    }
  }
}

void GaussianMixtureModeling::FloorVariance(
    std::vector<SymmetricMatrix>* covariance_matrices) const {
  for (int k(0); k < num_mixture_; ++k) {
    for (int l(0); l <= num_order_; ++l) {
      if ((*covariance_matrices)[k][l][l] < variance_floor_) {
        (*covariance_matrices)[k][l][l] = variance_floor_;
      }
    }
  }
}

bool GaussianMixtureModeling::Initialize(
    const std::vector<std::vector<double> >& input_vectors,
    std::vector<double>* weights,
    std::vector<std::vector<double> >* mean_vectors,
    std::vector<SymmetricMatrix>* covariance_matrices) const {
  // Initialize codebook.
  {
    mean_vectors->clear();
    StatisticsAccumulation statistics_accumulation(num_order_, 1);
    StatisticsAccumulation::Buffer buffer;

    for (const std::vector<double>& vector : input_vectors) {
      if (!statistics_accumulation.Run(vector, &buffer)) {
        return false;
      }
    }

    std::vector<double> mean;
    if (!statistics_accumulation.GetMean(buffer, &mean)) {
      return false;
    }
    mean_vectors->push_back(mean);
  }

  // Initialize mean vectors.
  std::vector<int> codebook_indices(input_vectors.size(), 0);
  if (2 <= num_mixture_) {
    const int num_iteration(1000);
    const double convergence_threshold(1e-5);
    const double splitting_factor(1e-5);
    const int seed(1);
    LindeBuzoGrayAlgorithm lbg(num_order_, 1, num_mixture_, 1, num_iteration,
                               convergence_threshold, splitting_factor, seed);
    if (!lbg.Run(input_vectors, mean_vectors, &codebook_indices)) {
      return false;
    }
  }

  // Count number of data for each cluster.
  const int num_data(static_cast<int>(input_vectors.size()));
  std::vector<int> num_data_in_cluster(num_mixture_);
  {
    int* src(&(codebook_indices[0]));
    int* dst(&(num_data_in_cluster[0]));
    for (int t(0); t < num_data; ++t) {
      ++dst[src[t]];
    }
  }

  // Initialize weights.
  {
    weights->resize(num_mixture_);
    for (int k(0); k < num_mixture_; ++k) {
      (*weights)[k] = static_cast<double>(num_data_in_cluster[k]) / num_data;
    }
    FloorWeight(weights);
  }

  // Initialize covariances.
  {
    covariance_matrices->resize(num_mixture_);
    for (int k(0); k < num_mixture_; ++k) {
      (*covariance_matrices)[k].Resize(num_order_ + 1);
      (*covariance_matrices)[k].Fill(0.0);
    }

    for (int t(0); t < num_data; ++t) {
      const double* x(&(input_vectors[t][0]));
      const int k(codebook_indices[t]);
      double* mu(&((*mean_vectors)[k][0]));
      for (int l(0); l <= num_order_; ++l) {
        const double diff1(x[l] - mu[l]);
        for (int m(is_diagonal_ ? l : 0); m <= l; ++m) {
          if (0.0 != mask_[l][m]) {
            const double diff2(x[m] - mu[m]);
            (*covariance_matrices)[k][l][m] += diff1 * diff2;
          }
        }
      }
    }

    for (int k(0); k < num_mixture_; ++k) {
      for (int l(0); l <= num_order_; ++l) {
        for (int m(is_diagonal_ ? l : 0); m <= l; ++m) {
          if (0.0 != mask_[l][m]) {
            (*covariance_matrices)[k][l][m] /= num_data_in_cluster[k];
          }
        }
      }
    }
    FloorVariance(covariance_matrices);
  }

  return true;
}

}  // namespace sptk
