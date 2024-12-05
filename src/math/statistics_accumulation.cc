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
#include <functional>  // std::minus, std::plus
#include <vector>      // std::vector

namespace sptk {

StatisticsAccumulation::StatisticsAccumulation(int num_order,
                                               int num_statistics_order,
                                               bool diagonal,
                                               bool numerically_stable)
    : num_order_(num_order),
      num_statistics_order_(num_statistics_order),
      diagonal_(diagonal),
      numerically_stable_(numerically_stable),
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

bool StatisticsAccumulation::GetFirst(
    const StatisticsAccumulation::Buffer& buffer,
    std::vector<double>* first) const {
  if (!is_valid_ || num_statistics_order_ < 1 || NULL == first) {
    return false;
  }

  *first = buffer.first_order_statistics_;

  return true;
}

bool StatisticsAccumulation::GetSecond(
    const StatisticsAccumulation::Buffer& buffer,
    SymmetricMatrix* second) const {
  if (!is_valid_ || num_statistics_order_ < 2 || NULL == second) {
    return false;
  }

  *second = buffer.second_order_statistics_;

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

  if (numerically_stable_) {
    const int n(buffer.zeroth_order_statistics_);
    std::transform(buffer.first_order_statistics_.begin(),
                   buffer.first_order_statistics_.end(), sum->begin(),
                   [n](double x) { return n * x; });
  } else {
    std::copy(buffer.first_order_statistics_.begin(),
              buffer.first_order_statistics_.end(), sum->begin());
  }

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

  if (numerically_stable_) {
    std::copy(buffer.first_order_statistics_.begin(),
              buffer.first_order_statistics_.end(), mean->begin());
  } else {
    const double z(1.0 / buffer.zeroth_order_statistics_);
    std::transform(buffer.first_order_statistics_.begin(),
                   buffer.first_order_statistics_.end(), mean->begin(),
                   [z](double x) { return z * x; });
  }

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

  const double z(1.0 / buffer.zeroth_order_statistics_);
  double* variance(&((*diagonal_covariance)[0]));

  if (numerically_stable_) {
    for (int i(0); i <= num_order_; ++i) {
      variance[i] = z * buffer.second_order_statistics_[i][i];
    }
  } else {
    std::vector<double> mean;
    if (!GetMean(buffer, &mean)) {
      return false;
    }
    const double* mu(&(mean[0]));
    for (int i(0); i <= num_order_; ++i) {
      variance[i] = z * buffer.second_order_statistics_[i][i] - mu[i] * mu[i];
    }
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
  if (!is_valid_ || num_statistics_order_ < 2 || diagonal_ ||
      buffer.zeroth_order_statistics_ <= 0 || NULL == full_covariance) {
    return false;
  }

  if (full_covariance->GetNumDimension() != num_order_ + 1) {
    full_covariance->Resize(num_order_ + 1);
  }

  const double z(1.0 / buffer.zeroth_order_statistics_);

  if (numerically_stable_) {
    for (int i(0); i <= num_order_; ++i) {
      for (int j(0); j <= i; ++j) {
        (*full_covariance)[i][j] = z * buffer.second_order_statistics_[i][j];
      }
    }
  } else {
    std::vector<double> mean;
    if (!GetMean(buffer, &mean)) {
      return false;
    }
    const double* mu(&(mean[0]));
    for (int i(0); i <= num_order_; ++i) {
      for (int j(0); j <= i; ++j) {
        (*full_covariance)[i][j] =
            z * buffer.second_order_statistics_[i][j] - mu[i] * mu[j];
      }
    }
  }

  return true;
}

bool StatisticsAccumulation::GetUnbiasedCovariance(
    const StatisticsAccumulation::Buffer& buffer,
    SymmetricMatrix* unbiased_covariance) const {
  if (!is_valid_ || num_statistics_order_ < 2 || diagonal_ ||
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
  if (!is_valid_ || num_statistics_order_ < 2 || diagonal_ ||
      NULL == correlation) {
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
  const std::size_t length(num_order_ + 1);
  if (!is_valid_ || data.size() != length || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (1 <= num_statistics_order_ &&
      buffer->first_order_statistics_.size() != length) {
    buffer->first_order_statistics_.resize(length);
  }
  if (2 <= num_statistics_order_ &&
      buffer->second_order_statistics_.GetNumDimension() !=
          static_cast<int>(length)) {
    buffer->second_order_statistics_.Resize(static_cast<int>(length));
  }
  if (numerically_stable_ && buffer->delta_.size() != length) {
    buffer->delta_.resize(length);
  }

  // Accumulate 0th order statistics.
  ++(buffer->zeroth_order_statistics_);

  // Compute delta.
  if (1 <= num_statistics_order_ && numerically_stable_) {
    std::transform(data.begin(), data.end(),
                   buffer->first_order_statistics_.begin(),
                   buffer->delta_.begin(), std::minus<double>());
  }
  const double* delta(&(buffer->delta_[0]));

  // Accumulate 2nd order statistics.
  if (2 <= num_statistics_order_) {
    if (numerically_stable_) {
      const double n(static_cast<double>(buffer->zeroth_order_statistics_));
      const double z((n - 1) / n);
      for (int i(0); i <= num_order_; ++i) {
        for (int j(diagonal_ ? i : 0); j <= i; ++j) {
          buffer->second_order_statistics_[i][j] += z * delta[i] * delta[j];
        }
      }
    } else {
      for (int i(0); i <= num_order_; ++i) {
        for (int j(diagonal_ ? i : 0); j <= i; ++j) {
          buffer->second_order_statistics_[i][j] += data[i] * data[j];
        }
      }
    }
  }

  // Accumulate 1st order statistics.
  if (1 <= num_statistics_order_) {
    if (numerically_stable_) {
      const double z(1.0 / buffer->zeroth_order_statistics_);
      std::transform(buffer->delta_.begin(), buffer->delta_.end(),
                     buffer->first_order_statistics_.begin(),
                     buffer->first_order_statistics_.begin(),
                     [z](double d, double a) { return a + z * d; });
    } else {
      std::transform(
          data.begin(), data.end(), buffer->first_order_statistics_.begin(),
          buffer->first_order_statistics_.begin(), std::plus<double>());
    }
  }

  return true;
}

bool StatisticsAccumulation::Merge(
    int num_data, const std::vector<double>& first,
    const SymmetricMatrix& second,
    StatisticsAccumulation::Buffer* buffer) const {
  if (!is_valid_ || num_data <= 0 || NULL == buffer) {
    return false;
  }

  const std::size_t length(num_order_ + 1);
  if (1 <= num_statistics_order_ && first.size() != length) {
    return false;
  }
  if (2 <= num_statistics_order_ &&
      second.GetNumDimension() != static_cast<int>(length)) {
    return false;
  }

  // Copy statistics if the buffer is empty.
  if (0 == buffer->zeroth_order_statistics_) {
    buffer->zeroth_order_statistics_ = num_data;
    if (!first.empty()) buffer->first_order_statistics_ = first;
    if (!second.Empty()) buffer->second_order_statistics_ = second;
    return true;
  }

  // Compute delta.
  if (numerically_stable_) {
    if (buffer->delta_.size() != length) {
      buffer->delta_.resize(length);
    }
    std::transform(buffer->first_order_statistics_.begin(),
                   buffer->first_order_statistics_.end(), first.begin(),
                   buffer->delta_.begin(), std::minus<double>());
  }
  const double* delta(&(buffer->delta_[0]));

  const double m(static_cast<double>(buffer->zeroth_order_statistics_));
  const double n(static_cast<double>(num_data));
  const double mpn(m + n);
  const double mn(m * n);
  buffer->zeroth_order_statistics_ += num_data;

  // Merge 2nd order statistics.
  if (!second.Empty()) {
    if (numerically_stable_) {
      const double c(mn / mpn);
      for (int i(0); i <= num_order_; ++i) {
        for (int j(diagonal_ ? i : 0); j <= i; ++j) {
          const double term1(second[i][j]);
          const double term2(c * delta[i] * delta[j]);
          buffer->second_order_statistics_[i][j] += term1 + term2;
        }
      }
    } else {
      for (int i(0); i <= num_order_; ++i) {
        for (int j(diagonal_ ? i : 0); j <= i; ++j) {
          buffer->second_order_statistics_[i][j] += second[i][j];
        }
      }
    }
  }

  // Merge 1st order statistics.
  if (!first.empty()) {
    if (numerically_stable_) {
      const double c(m / mpn);
      std::transform(buffer->delta_.begin(), buffer->delta_.end(),
                     first.begin(), buffer->first_order_statistics_.begin(),
                     [c](double x, double y) { return c * x + y; });
    } else {
      std::transform(
          first.begin(), first.end(), buffer->first_order_statistics_.begin(),
          buffer->first_order_statistics_.begin(), std::plus<double>());
    }
  }

  return true;
}

}  // namespace sptk
