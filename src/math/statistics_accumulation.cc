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

#include "SPTK/math/statistics_accumulation.h"

#include <algorithm>   // std::copy, std::transform
#include <cmath>       // std::sqrt
#include <cstddef>     // std::size_t
#include <functional>  // std::plus

namespace sptk {

StatisticsAccumulation::StatisticsAccumulation(int num_order,
                                               int num_statistics_order)
    : num_order_(num_order),
      num_statistics_order_(num_statistics_order),
      is_valid_(true) {
  if (num_order_ < 0 || num_statistics_order_ < 0 ||
      2 < num_statistics_order_) {
    is_valid_ = false;
    return;
  }
}

bool StatisticsAccumulation::GetNumData(
    const StatisticsAccumulation::Buffer& buffer, int* num_data) const {
  if (!is_valid_ || NULL == num_data) {
    return false;
  }

  *num_data = buffer.zeroth_order_statistics_;

  return true;
}

bool StatisticsAccumulation::GetSum(
    const StatisticsAccumulation::Buffer& buffer,
    std::vector<double>* sum) const {
  if (!is_valid_ || num_statistics_order_ < 1 || NULL == sum) {
    return false;
  }

  if (sum->size() != static_cast<std::size_t>(num_order_ + 1)) {
    sum->resize(num_order_ + 1);
  }

  std::copy(buffer.first_order_statistics_.begin(),
            buffer.first_order_statistics_.end(), sum->begin());

  return true;
}

bool StatisticsAccumulation::GetMean(
    const StatisticsAccumulation::Buffer& buffer,
    std::vector<double>* mean) const {
  if (!is_valid_ || num_statistics_order_ < 1 ||
      buffer.zeroth_order_statistics_ <= 0 || NULL == mean) {
    return false;
  }

  if (mean->size() != static_cast<std::size_t>(num_order_ + 1)) {
    mean->resize(num_order_ + 1);
  }

  const double z(1.0 / buffer.zeroth_order_statistics_);
  std::transform(buffer.first_order_statistics_.begin(),
                 buffer.first_order_statistics_.end(), mean->begin(),
                 [z](double x) { return x * z; });

  return true;
}

bool StatisticsAccumulation::GetDiagonalCovariance(
    const StatisticsAccumulation::Buffer& buffer,
    std::vector<double>* diagonal_covariance) const {
  if (!is_valid_ || num_statistics_order_ < 2 ||
      buffer.zeroth_order_statistics_ <= 0 || NULL == diagonal_covariance) {
    return false;
  }

  if (diagonal_covariance->size() != static_cast<std::size_t>(num_order_ + 1)) {
    diagonal_covariance->resize(num_order_ + 1);
  }

  std::vector<double> mean;
  if (!GetMean(buffer, &mean)) {
    return false;
  }

  const double z(1.0 / buffer.zeroth_order_statistics_);
  const double* mu(&(mean[0]));
  double* variance(&((*diagonal_covariance)[0]));
  for (int i(0); i <= num_order_; ++i) {
    variance[i] = z * buffer.second_order_statistics_[i][i] - mu[i] * mu[i];
  }

  return true;
}

bool StatisticsAccumulation::GetStandardDeviation(
    const StatisticsAccumulation::Buffer& buffer,
    std::vector<double>* standard_deviation) const {
  if (!is_valid_ || num_statistics_order_ < 2 || NULL == standard_deviation) {
    return false;
  }

  if (!GetDiagonalCovariance(buffer, standard_deviation)) {
    return false;
  }

  std::transform(standard_deviation->begin(), standard_deviation->end(),
                 standard_deviation->begin(),
                 [](double x) { return std::sqrt(x); });

  return true;
}

bool StatisticsAccumulation::GetFullCovariance(
    const StatisticsAccumulation::Buffer& buffer,
    SymmetricMatrix* full_covariance) const {
  if (!is_valid_ || num_statistics_order_ < 2 ||
      buffer.zeroth_order_statistics_ <= 0 || NULL == full_covariance) {
    return false;
  }

  if (full_covariance->GetNumDimension() != num_order_ + 1) {
    full_covariance->Resize(num_order_ + 1);
  }

  std::vector<double> mean;
  if (!GetMean(buffer, &mean)) {
    return false;
  }

  const double z(1.0 / buffer.zeroth_order_statistics_);
  const double* mu(&(mean[0]));
  for (int i(0); i <= num_order_; ++i) {
    for (int j(0); j <= i; ++j) {
      (*full_covariance)[i][j] =
          z * buffer.second_order_statistics_[i][j] - mu[i] * mu[j];
    }
  }

  return true;
}

bool StatisticsAccumulation::GetUnbiasedCovariance(
    const StatisticsAccumulation::Buffer& buffer,
    SymmetricMatrix* unbiased_covariance) const {
  if (!is_valid_ || num_statistics_order_ < 2 ||
      buffer.zeroth_order_statistics_ <= 1 || NULL == unbiased_covariance) {
    return false;
  }

  if (!GetFullCovariance(buffer, unbiased_covariance)) {
    return false;
  }

  const double z(static_cast<double>(buffer.zeroth_order_statistics_) /
                 (buffer.zeroth_order_statistics_ - 1));
  for (int i(0); i <= num_order_; ++i) {
    for (int j(0); j <= i; ++j) {
      (*unbiased_covariance)[i][j] *= z;
    }
  }

  return true;
}

bool StatisticsAccumulation::GetCorrelation(
    const StatisticsAccumulation::Buffer& buffer,
    SymmetricMatrix* correlation) const {
  if (!is_valid_ || num_statistics_order_ < 2 || NULL == correlation) {
    return false;
  }

  std::vector<double> standard_deviation;
  if (!GetStandardDeviation(buffer, &standard_deviation)) {
    return false;
  }
  if (!GetFullCovariance(buffer, correlation)) {
    return false;
  }

  const double* sigma(&(standard_deviation[0]));
  for (int i(0); i <= num_order_; ++i) {
    for (int j(0); j <= i; ++j) {
      (*correlation)[i][j] = (*correlation)[i][j] / (sigma[i] * sigma[j]);
    }
  }

  return true;
}

void StatisticsAccumulation::Clear(
    StatisticsAccumulation::Buffer* buffer) const {
  if (!is_valid_ || NULL != buffer) buffer->Clear();
}

bool StatisticsAccumulation::Run(const std::vector<double>& data,
                                 StatisticsAccumulation::Buffer* buffer) const {
  // Check inputs.
  const int length(num_order_ + 1);
  if (!is_valid_ || data.size() != static_cast<std::size_t>(length) ||
      NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (1 <= num_statistics_order_ && buffer->first_order_statistics_.size() !=
                                        static_cast<std::size_t>(length)) {
    buffer->first_order_statistics_.resize(length);
  }
  if (2 <= num_statistics_order_ &&
      buffer->second_order_statistics_.GetNumDimension() != length) {
    buffer->second_order_statistics_.Resize(length);
  }

  // Accumulate 0th order statistics.
  ++(buffer->zeroth_order_statistics_);

  // Accumulate 1st order statistics.
  if (1 <= num_statistics_order_) {
    std::transform(
        data.begin(), data.end(), buffer->first_order_statistics_.begin(),
        buffer->first_order_statistics_.begin(), std::plus<double>());
  }

  // Accumulate 2nd order statistics.
  if (2 <= num_statistics_order_) {
    for (int i(0); i < length; ++i) {
      for (int j(0); j <= i; ++j) {
        buffer->second_order_statistics_[i][j] += data[i] * data[j];
      }
    }
  }

  return true;
}

}  // namespace sptk
