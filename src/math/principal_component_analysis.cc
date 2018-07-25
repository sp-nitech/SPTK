// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2018  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

#include "SPTK/math/principal_component_analysis.h"

#include <algorithm>  // std::sort, std::swap
#include <cmath>      // std::fabs, std::sqrt
#include <cstddef>    // std::size_t
#include <numeric>    // std::iota

namespace sptk {

PrincipalComponentAnalysis::PrincipalComponentAnalysis(
    int num_order, int num_iteration, double convergence_threshold)
    : num_order_(num_order),
      num_iteration_(num_iteration),
      convergence_threshold_(convergence_threshold),
      accumulator_(num_order, 2),
      is_valid_(true) {
  if (num_order_ < 0 || num_iteration_ <= 0 || convergence_threshold_ < 0.0 ||
      !accumulator_.IsValid()) {
    is_valid_ = false;
  }
}

bool PrincipalComponentAnalysis::Run(
    const std::vector<std::vector<double> >& input_vectors,
    std::vector<double>* mean_vector, std::vector<double>* eigenvalues,
    Matrix* eigenvector_matrix,
    PrincipalComponentAnalysis::Buffer* buffer) const {
  if (!is_valid_ || NULL == mean_vector || NULL == eigenvalues ||
      NULL == eigenvector_matrix || NULL == buffer) {
    return false;
  }

  // prepare memory
  const int length(num_order_ + 1);
  if (eigenvector_matrix->GetNumRow() != length ||
      eigenvector_matrix->GetNumColumn() != length) {
    eigenvector_matrix->Resize(length, length);
  }
  if (eigenvalues->size() != static_cast<std::size_t>(length)) {
    eigenvalues->resize(length);
  }
  if (buffer->eigenvalue_order_.size() != static_cast<std::size_t>(length)) {
    buffer->eigenvalue_order_.resize(length);
  }

  // calculate statistics
  accumulator_.Clear(&buffer->accumulator_buffer_);
  for (std::vector<double> input_vector : input_vectors) {
    if (!accumulator_.Run(input_vector, &buffer->accumulator_buffer_)) {
      return false;
    }
  }
  if (!accumulator_.GetMean(buffer->accumulator_buffer_, mean_vector)) {
    return false;
  }
  if (!accumulator_.GetFullCovariance(buffer->accumulator_buffer_,
                                      &buffer->a_)) {
    return false;
  }

  // initialize with identity matrix
  for (int i(0); i < length; ++i) {
    for (int j(0); j < length; ++j) {
      (*eigenvector_matrix)[i][j] = (i == j) ? 1.0 : 0.0;
    }
  }

  // solve a set of linear equations using Jacobi iterative method
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
    // the following equation can be derived from
    //   theta = 1/2 * atan(2 * a_pq / (a_pp - a_qq)),
    // using the double-angle formulas and the quadratic formula
    const double tan_theta(t2 / (t1 + std::sqrt(t1 * t1 + t2 * t2)));
    const double cos_theta(std::sqrt(1.0 / (tan_theta * tan_theta + 1.0)));
    const double sin_theta((a_pp < a_qq) ? tan_theta * cos_theta
                                         : -tan_theta * cos_theta);

    // update eigenvectors
    for (int i(0); i < length; ++i) {
      const double w1((*eigenvector_matrix)[p][i]);
      const double w2((*eigenvector_matrix)[q][i]);
      (*eigenvector_matrix)[p][i] = w1 * cos_theta - w2 * sin_theta;
      (*eigenvector_matrix)[q][i] = w1 * sin_theta + w2 * cos_theta;
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

  // sort eigenvalues in descending order
  std::iota(buffer->eigenvalue_order_.begin(), buffer->eigenvalue_order_.end(),
            0);
  std::sort(
      buffer->eigenvalue_order_.begin(), buffer->eigenvalue_order_.end(),
      [&buffer](int i, int j) { return buffer->a_[j][j] < buffer->a_[i][i]; });
  for (int i(0); i < length; ++i) {
    (*eigenvalues)[i] =
        buffer->a_[buffer->eigenvalue_order_[i]][buffer->eigenvalue_order_[i]];
  }

  // swap eigenvectors
  for (int i(0); i < num_order_; ++i) {
    int index(i);
    while (index < length) {
      if (buffer->eigenvalue_order_[index] == i) {
        buffer->eigenvalue_order_[index] = buffer->eigenvalue_order_[i];
        break;
      }
      ++index;
    }

    const int j(buffer->eigenvalue_order_[index]);
    if (j != i) {
      for (int k(0); k < length; ++k) {
        std::swap((*eigenvector_matrix)[j][k], (*eigenvector_matrix)[i][k]);
      }
    }
  }

  return true;
}

}  // namespace sptk
