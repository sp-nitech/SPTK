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
//                1996-2020  Nagoya Institute of Technology          //
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

#include "SPTK/generation/nonrecursive_maximum_likelihood_parameter_generation.h"

#include <algorithm>  // std::count
#include <cstddef>    // std::size_t

namespace {

bool CheckSize(const std::vector<std::vector<double> >& vectors, int size) {
  const int outer_size(static_cast<int>(vectors.size()));
  for (int i(0); i < outer_size; ++i) {
    const int inner_size(static_cast<int>(vectors[i].size()));
    if (inner_size != size) {
      return false;
    }
  }
  return true;
}

bool CheckSize(const std::vector<sptk::SymmetricMatrix>& matrices, int size) {
  const int outer_size(matrices.size());
  for (int i(0); i < outer_size; ++i) {
    const int inner_size(matrices[i].GetNumDimension());
    if (inner_size != size) {
      return false;
    }
  }
  return true;
}

bool CheckRange(int index, int max, const double* array, bool reverse) {
  return (-max <= index && index <= max && ((reverse && 0.0 != array[-index]) ||
                                            (!reverse && 0.0 != array[index])))
             ? true
             : false;
}

}  // namespace

namespace sptk {

NonrecursiveMaximumLikelihoodParameterGeneration::
    NonrecursiveMaximumLikelihoodParameterGeneration(
        int num_order,
        const std::vector<std::vector<double> >& window_coefficients,
        bool use_magic_number, double magic_number)
    : num_order_(num_order),
      window_coefficients_(window_coefficients),
      use_magic_number_(use_magic_number),
      magic_number_(magic_number),
      is_valid_(true),
      max_half_window_width_(0) {
  if (num_order < 0) {
    is_valid_ = false;
    return;
  }

  // Insert window coeffients for static components.
  window_coefficients_.insert(window_coefficients_.begin(), {1.0});

  for (std::vector<std::vector<double> >::iterator itr(
           window_coefficients_.begin());
       itr != window_coefficients_.end(); ++itr) {
    const int window_width(static_cast<int>(itr->size()));
    if (0 == window_width % 2) {
      itr->push_back(0.0);
    }
    const int half_window_width(window_width / 2);
    if (max_half_window_width_ < half_window_width) {
      max_half_window_width_ = half_window_width;
    }
  }
}

bool NonrecursiveMaximumLikelihoodParameterGeneration::Run(
    const std::vector<std::vector<double> >& mean_vectors,
    const std::vector<std::vector<double> >& variance_vectors,
    std::vector<std::vector<double> >* smoothed_static_parameters) const {
  if (!is_valid_ || variance_vectors.empty()) {
    return false;
  }

  const int sequence_length(static_cast<int>(variance_vectors.size()));
  std::vector<sptk::SymmetricMatrix> covariance_matrices(
      sequence_length, sptk::SymmetricMatrix(variance_vectors[0].size()));
  for (int t(0); t < sequence_length; ++t) {
    if (!covariance_matrices[t].SetDiagonal(variance_vectors[t])) {
      return false;
    }
  }
  return Run(mean_vectors, covariance_matrices, smoothed_static_parameters);
}

bool NonrecursiveMaximumLikelihoodParameterGeneration::Run(
    const std::vector<std::vector<double> >& mean_vectors,
    const std::vector<sptk::SymmetricMatrix>& covariance_matrices,
    std::vector<std::vector<double> >* smoothed_static_parameters) const {
  // Check inputs.
  if (!is_valid_ || mean_vectors.empty() ||
      mean_vectors.size() != covariance_matrices.size() ||
      NULL == smoothed_static_parameters) {
    return false;
  }

  const int num_window(static_cast<int>(window_coefficients_.size()));
  const int static_size(num_order_ + 1);
  const int length(static_size * num_window);
  if (!CheckSize(mean_vectors, length) ||
      !CheckSize(covariance_matrices, length)) {
    return false;
  }

  // Store positions that contain a magic number.
  const int sequence_length(static_cast<int>(mean_vectors.size()));
  std::vector<bool> is_continuous(sequence_length, true);
  if (use_magic_number_) {
    for (int absolute_t(0); absolute_t < sequence_length; ++absolute_t) {
      if (magic_number_ == mean_vectors[absolute_t][0]) {
        is_continuous[absolute_t] = false;
      }
    }
  }
  const int continuous_length(
      std::count(is_continuous.begin(), is_continuous.end(), true));

  // Prepare memories.
  const int max_window_width(2 * max_half_window_width_ + 1);
  const int wuw_height(static_size * continuous_length);
  const int wuw_width(static_size * max_window_width);
  std::vector<std::vector<double> > mseq(continuous_length,
                                         std::vector<double>(length));
  std::vector<sptk::SymmetricMatrix> vseq(continuous_length,
                                          sptk::SymmetricMatrix(length));
  std::vector<double> wum(wuw_height);
  std::vector<std::vector<double> > wuw(wuw_height,
                                        std::vector<double>(wuw_width));
  std::vector<double> gg(wuw_height);
  std::vector<double> cc(wuw_height);
  {
    if (smoothed_static_parameters->size() !=
        static_cast<std::size_t>(sequence_length)) {
      smoothed_static_parameters->resize(sequence_length);
    }
    for (int t(0); t < sequence_length; ++t) {
      if ((*smoothed_static_parameters)[t].size() !=
          static_cast<std::size_t>(static_size)) {
        (*smoothed_static_parameters)[t].resize(static_size);
      }
    }
  }

  // Set mseq and vseq.
  for (int absolute_t(0), t(0); absolute_t < sequence_length; ++absolute_t) {
    if (!is_continuous[absolute_t]) continue;

    SymmetricMatrix precision;
    if (!covariance_matrices[absolute_t].Invert(&precision)) {
      return false;
    }
    const double* mean(&(mean_vectors[absolute_t][0]));

    for (int k(0); k < length; ++k) {
      // Check boundary.
      bool is_boundary(false);
      {
        const int d(k / static_size);
        const int half_window_width(
            (static_cast<int>(window_coefficients_[d].size()) - 1) / 2);
        for (int j(-half_window_width); j <= half_window_width; ++j) {
          const int biased_t(t + j);
          const int biased_absolute_t(absolute_t + j);
          const double* window_coefficients(
              &(window_coefficients_[d][half_window_width]));
          if (biased_t < 0 || continuous_length <= biased_t ||
              (0.0 != window_coefficients[j] && 0 <= biased_absolute_t &&
               biased_absolute_t < sequence_length &&
               !is_continuous[biased_absolute_t])) {
            is_boundary = true;
          }
        }
      }

      if (!is_boundary) {
        const double p(precision[k][k]);
        mseq[t][k] = p * mean[k];
        vseq[t][k][k] = p;
        for (int l(0); l < k; ++l) {
          const double p(precision[k][l]);
          if (0.0 != p) {
            mseq[t][k] += p * mean[l];
            mseq[t][l] += p * mean[k];
            vseq[t][k][l] = p;
          }
        }
      }
    }

    // Update counter.
    ++t;
  }

  // Calculate WUM and WUW.
  for (int t(0); t < continuous_length; ++t) {
    for (int m(0); m < static_size; ++m) {
      const int tau(static_size * t + m);
      for (int n(0); n < static_size; ++n) {
        for (int d(0); d < num_window; ++d) {
          const int half_window_width(
              (static_cast<int>(window_coefficients_[d].size()) - 1) / 2);
          const double* window_coefficients(
              &(window_coefficients_[d][half_window_width]));
          for (int j(-max_half_window_width_); j <= max_half_window_width_;
               ++j) {
            const int biased_t(t + j);
            if (biased_t < 0 || continuous_length <= biased_t) continue;

            // Accumulate W'U^{-1}M.
            if (0 == n &&
                CheckRange(j, half_window_width, window_coefficients, true)) {
              wum[tau] += (window_coefficients[-j] *
                           mseq[biased_t][d * static_size + m]);
            }

            // Accumulate W'U^{-1}W.
            double wu(0.0);
            for (int c(0); c < num_window; ++c) {
              const double u(
                  vseq[biased_t][static_size * c + m][static_size * d + n]);
              const int half_window_width2(
                  (static_cast<int>(window_coefficients_[c].size()) - 1) / 2);
              const double* window_coefficients2(
                  &(window_coefficients_[c][half_window_width2]));
              if (CheckRange(j, half_window_width2, window_coefficients2,
                             true) &&
                  0.0 != u) {
                wu += window_coefficients2[-j] * u;
              }
            }
            if (0.0 != wu) {
              for (int k(0); k < max_window_width && t + k < continuous_length;
                   ++k) {
                const int index(static_size * k + n - m);
                if (CheckRange(k - j, half_window_width, window_coefficients,
                               false) &&
                    0 <= index) {
                  wuw[tau][index] += wu * window_coefficients[k - j];
                }
              }
            }
          }
        }
      }
    }
  }

  // Compute Cholesky factor.
  for (int t(0); t < wuw_height; ++t) {
    for (int i(1); i < wuw_width && i <= t; ++i) {
      wuw[t][0] -= wuw[t - i][i] * wuw[t - i][i] * wuw[t - i][0];
    }

    const double z(1.0 / wuw[t][0]);
    for (int i(1); i < wuw_width; ++i) {
      for (int j(1); i + j < wuw_width && j <= t; ++j) {
        wuw[t][i] -= wuw[t - j][j] * wuw[t - j][i + j] * wuw[t - j][0];
      }
      wuw[t][i] *= z;
    }
  }

  // Forward substitution to solve a set of linear equations.
  {
    double* r(&(wum[0]));
    double* g(&(gg[0]));
    for (int t(0); t < wuw_height; ++t) {
      g[t] = r[t];
      for (int i(1); i < wuw_width && i <= t; ++i) {
        g[t] -= wuw[t - i][i] * g[t - i];
      }
    }
  }

  // Backward substitution to solve a set of linear equations.
  {
    double* g(&(gg[0]));
    double* c(&(cc[0]));
    for (int t(wuw_height - 1); 0 <= t; --t) {
      c[t] = g[t] / wuw[t][0];
      for (int i(1); i < wuw_width && t + i < wuw_height; ++i) {
        c[t] -= wuw[t][i] * c[t + i];
      }
    }
  }

  // Store generated parameters.
  {
    double* c(&(cc[0]));
    int u(0);
    for (int absolute_t(0); absolute_t < sequence_length; ++absolute_t) {
      double* C(&((*smoothed_static_parameters)[absolute_t][0]));
      if (is_continuous[absolute_t]) {
        for (int k(0); k < static_size; ++k) {
          C[k] = c[u++];
        }
      } else {
        for (int k(0); k < static_size; ++k) {
          C[k] = magic_number_;
        }
      }
    }
  }

  return true;
}

}  // namespace sptk
