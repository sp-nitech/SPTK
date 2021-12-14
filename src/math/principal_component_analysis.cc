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

#include "SPTK/math/principal_component_analysis.h"

#include <algorithm>  // std::sort, std::swap
#include <cmath>      // std::fabs, std::sqrt
#include <cstddef>    // std::size_t
#include <numeric>    // std::iota

namespace sptk {

PrincipalComponentAnalysis::PrincipalComponentAnalysis(
    int num_order, int num_iteration, double convergence_threshold,
    CovarianceType covariance_type)
    : num_order_(num_order),
      num_iteration_(num_iteration),
      convergence_threshold_(convergence_threshold),
      covariance_type_(covariance_type),
      accumulation_(num_order, 2),
      is_valid_(true) {
  if (num_order_ < 0 || num_iteration_ <= 0 || convergence_threshold_ < 0.0 ||
      kNumCovarianceTypes == covariance_type_ || !accumulation_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool PrincipalComponentAnalysis::Run(
    const std::vector<std::vector<double> >& input_vectors,
    std::vector<double>* mean_vector, std::vector<double>* eigenvalues,
    Matrix* eigenvectors, PrincipalComponentAnalysis::Buffer* buffer) const {
  // Check inputs.
  if (!is_valid_ || NULL == mean_vector || NULL == eigenvalues ||
      NULL == eigenvectors || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  const int length(num_order_ + 1);
  if (eigenvalues->size() != static_cast<std::size_t>(length)) {
    eigenvalues->resize(length);
  }
  if (eigenvectors->GetNumRow() != length ||
      eigenvectors->GetNumColumn() != length) {
    eigenvectors->Resize(length, length);
  }
  if (buffer->order_of_eigenvalue_.size() != static_cast<std::size_t>(length)) {
    buffer->order_of_eigenvalue_.resize(length);
  }

  // Calculate statistics.
  accumulation_.Clear(&buffer->buffer_for_accumulation);
  for (const std::vector<double>& input_vector : input_vectors) {
    if (!accumulation_.Run(input_vector, &buffer->buffer_for_accumulation)) {
      return false;
    }
  }
  if (!accumulation_.GetMean(buffer->buffer_for_accumulation, mean_vector)) {
    return false;
  }
  switch (covariance_type_) {
    case kSampleCovariance: {
      if (!accumulation_.GetFullCovariance(buffer->buffer_for_accumulation,
                                           &buffer->a_)) {
        return false;
      }
      break;
    }
    case kUnbiasedCovariance: {
      if (!accumulation_.GetUnbiasedCovariance(buffer->buffer_for_accumulation,
                                               &buffer->a_)) {
        return false;
      }
      break;
    }
    case kCorrelation: {
      if (!accumulation_.GetCorrelation(buffer->buffer_for_accumulation,
                                        &buffer->a_)) {
        return false;
      }
      break;
    }
    default: {
      return false;
    }
  }

  // Initialize eigenvector matrix with identity matrix.
  eigenvectors->FillDiagonal(1.0);

  // Solve a set of linear equations using Jacobi iterative method.
  for (int n(0); n < num_iteration_; ++n) {
    int p(0);
    int q(0);
    {
      double off_diagonal_max(0.0);
      for (int i(0); i < length; ++i) {
        for (int j(i + 1); j < length; ++j) {
          const double val(std::fabs(buffer->a_[i][j]));
          if (off_diagonal_max < val) {
            off_diagonal_max = val;
            p = i;
            q = j;
          }
        }
      }
      if (off_diagonal_max <= convergence_threshold_) {
        break;
      }
    }

    const double a_pp(buffer->a_[p][p]);
    const double a_qq(buffer->a_[q][q]);
    const double a_pq(buffer->a_[p][q]);

    const double t1(std::fabs(a_pp - a_qq));
    const double t2(2.0 * a_pq);
    // The following equation can be derived from
    //   theta = 1/2 * atan(2 * a_pq / (a_pp - a_qq)),
    // using the double-angle formulas and the quadratic formula.
    const double tan_theta(t2 / (t1 + std::sqrt(t1 * t1 + t2 * t2)));
    const double cos_theta(std::sqrt(1.0 / (tan_theta * tan_theta + 1.0)));
    const double sin_theta((a_pp < a_qq) ? tan_theta * cos_theta
                                         : -tan_theta * cos_theta);

    // Update eigenvectors.
    for (int i(0); i < length; ++i) {
      const double w1((*eigenvectors)[p][i]);
      const double w2((*eigenvectors)[q][i]);
      (*eigenvectors)[p][i] = w1 * cos_theta - w2 * sin_theta;
      (*eigenvectors)[q][i] = w1 * sin_theta + w2 * cos_theta;
    }

    for (int i(0); i < length; ++i) {
      if (p != i && q != i) {
        const double w1(buffer->a_[p][i]);
        const double w2(buffer->a_[q][i]);
        buffer->a_[p][i] = w1 * cos_theta - w2 * sin_theta;
        buffer->a_[q][i] = w1 * sin_theta + w2 * cos_theta;
      }
    }
    buffer->a_[p][p] = (a_pp * cos_theta * cos_theta) +
                       (a_qq * sin_theta * sin_theta) -
                       (2.0 * a_pq * cos_theta * sin_theta);
    buffer->a_[q][q] = a_pp + a_qq - buffer->a_[p][p];
    buffer->a_[p][q] = 0.0;
  }

  // Sort eigenvalues in descending order.
  std::iota(buffer->order_of_eigenvalue_.begin(),
            buffer->order_of_eigenvalue_.end(), 0);
  std::sort(
      buffer->order_of_eigenvalue_.begin(), buffer->order_of_eigenvalue_.end(),
      [&buffer](int i, int j) { return buffer->a_[j][j] < buffer->a_[i][i]; });
  for (int i(0); i < length; ++i) {
    const int j(buffer->order_of_eigenvalue_[i]);
    (*eigenvalues)[i] = buffer->a_[j][j];
  }

  // Swap eigenvectors.
  for (int i(0); i < num_order_; ++i) {
    int index(i);
    for (; index < length; ++index) {
      if (buffer->order_of_eigenvalue_[index] == i) {
        buffer->order_of_eigenvalue_[index] = buffer->order_of_eigenvalue_[i];
        break;
      }
    }

    const int j(buffer->order_of_eigenvalue_[index]);
    if (j != i) {
      for (int k(0); k < length; ++k) {
        std::swap((*eigenvectors)[j][k], (*eigenvectors)[i][k]);
      }
    }
  }

  return true;
}

}  // namespace sptk
