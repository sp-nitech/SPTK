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

#include "SPTK/utils/misc_utils.h"

#include <algorithm>  // std::fill, std::max, std::min, std::transform
#include <cfloat>     // DBL_MAX
#include <cmath>      // std::atan, std::log, std::pow, std::sqrt, etc.
#include <cstddef>    // std::size_t

#include "SPTK/math/real_valued_fast_fourier_transform.h"
#include "SPTK/utils/sptk_utils.h"
#include "SPTK/window/kaiser_window.h"

namespace sptk {

bool ComputePercentagePointOfStandardNormalDistribution(
    double probability, double* percentage_point) {
  if (probability <= 0.0 || 1.0 <= probability || NULL == percentage_point) {
    return false;
  }

  if (0.5 == probability) {
    *percentage_point = 0.0;
    return true;
  }

  const double y(-std::log(4.0 * probability * (1.0 - probability)));
  const std::vector<double> parameters{
      0.1570796288e+1,  0.3706987906e-1,  -0.8364353589e-3, -0.2250947176e-3,
      0.6841218299e-5,  0.5824238515e-5,  -0.1045274970e-5, 0.8360937017e-7,
      -0.3231081277e-8, 0.3657763036e-10, 0.6936233982e-12};

  double sum(0.0);
  for (int i(0); i < static_cast<int>(parameters.size()); ++i) {
    sum += parameters[i] * std::pow(y, i);
  }
  *percentage_point =
      (0.5 < probability) ? -std::sqrt(sum * y) : std::sqrt(sum * y);
  return true;
}

bool ComputeProbabilityOfTDistribution(double percentage_point,
                                       int degrees_of_freedom,
                                       double* probability) {
  if (degrees_of_freedom <= 0 || NULL == probability) {
    return false;
  }

  const double cosine_squared(
      degrees_of_freedom /
      (degrees_of_freedom + percentage_point * percentage_point));
  const double sine((percentage_point < 0.0) ? -std::sqrt(1.0 - cosine_squared)
                                             : std::sqrt(1.0 - cosine_squared));

  double sum(0.0);
  double prod(sine);
  for (int i(degrees_of_freedom % 2 + 2); i <= degrees_of_freedom; i += 2) {
    sum += prod;
    prod *= (i - 1) * cosine_squared / i;
  }
  if (0 == degrees_of_freedom % 2) {
    *probability = (1.0 + sum) / 2.0;
  } else {
    const double theta(
        std::atan(percentage_point / std::sqrt(degrees_of_freedom)));
    *probability = 0.5 + (sum * std::sqrt(cosine_squared) + theta) / sptk::kPi;
  }

  return true;
}

bool ComputePercentagePointOfTDistribution(double probability,
                                           int degrees_of_freedom,
                                           double* percentage_point) {
  if (probability <= 0.0 || 1.0 <= probability || degrees_of_freedom <= 0 ||
      NULL == percentage_point) {
    return false;
  }

  if (0.5 == probability) {
    *percentage_point = 0.0;
    return true;
  }

  double z;
  if (!ComputePercentagePointOfStandardNormalDistribution(probability, &z)) {
    return false;
  }
  const double z_squared(z * z);
  const std::vector<int> parameters{1,     1,     4,     5,     16,  3,   96,
                                    3,     19,    17,    -15,   384, 79,  776,
                                    1482,  -1920, -945,  92160, 27,  339, 930,
                                    -1782, -765,  17955, 368640};

  std::vector<double> term(5);
  for (int i(0), index(0); i < static_cast<int>(term.size()); ++i) {
    term[i] = parameters[index++];
    for (int j(0); j <= i; ++j) {
      term[i] = term[i] * z_squared + parameters[index++];
    }
    term[i] /= parameters[index++];
  }

  const double inverse_degrees_of_freedom(1.0 / degrees_of_freedom);
  *percentage_point = 0.0;
  for (int i(term.size() - 1); 0 <= i; --i) {
    *percentage_point =
        (*percentage_point + term[i]) * inverse_degrees_of_freedom;
  }
  *percentage_point = (*percentage_point + 1.0) * z;

  if (degrees_of_freedom <=
      std::pow(std::log10(1.0 - probability), 2.0) + 3.0) {
    const double n(degrees_of_freedom + 1);
    double computed_probability;
    if (!ComputeProbabilityOfTDistribution(
            *percentage_point, degrees_of_freedom, &computed_probability)) {
      return false;
    }
    const double percentage_point_squared(*percentage_point *
                                          *percentage_point);
    const double tmp1(
        std::log(n / (degrees_of_freedom + percentage_point_squared)));
    const double tmp2(std::log(degrees_of_freedom / (n * sptk::kTwoPi)));
    const double tmp3(1.0 / (6.0 * n * degrees_of_freedom));
    const double delta((computed_probability - (1.0 - probability)) /
                       std::exp((n * tmp1 + tmp2 - 1.0 - tmp3) / 2.0));
    *percentage_point -= delta;
  }
  return true;
}

bool MakePseudoQuadratureMirrorFilterBanks(
    bool inverse, int num_subband, int num_filter_order, double attenuation,
    int num_iteration, double convergence_threshold, double initial_step_size,
    std::vector<std::vector<double> >* filter_banks, bool* is_converged) {
  if (0 == num_subband || num_filter_order <= 1 || attenuation <= 0.0 ||
      0 == num_iteration || convergence_threshold < 0.0 ||
      initial_step_size <= 0.0 || NULL == filter_banks) {
    return false;
  }

  if (is_converged) *is_converged = false;

  // Design prototype filter.
  const int filter_size(num_filter_order + 1);
  std::vector<double> prototype_filter(filter_size);
  {
    // Make Kaiser window.
    const sptk::KaiserWindow kaiser_window(
        filter_size, sptk::KaiserWindow::AttenuationToBeta(attenuation), true);
    const std::vector<double>& window(kaiser_window.Get());

    // Prepare FFT.
    const int fft_size(sptk::NextPow(filter_size));
    sptk::RealValuedFastFourierTransform fft(num_filter_order, fft_size);
    sptk::RealValuedFastFourierTransform::Buffer buffer_for_fft;
    if (!fft.IsValid()) {
      return false;
    }

    std::vector<double> ideal_filter(filter_size);
    std::vector<double> real;
    std::vector<double> imag;
    const int index(fft_size / (4 * num_subband));

    // Initialize.
    double omega(sptk::kPi / (2 * num_subband));
    double step_size(initial_step_size);
    double best_abs_error(DBL_MAX);

    for (int i(0); i < num_iteration; ++i) {
      // Make ideal filter.
      {
        double* h(&(ideal_filter[0]));
        for (int n(0); n < filter_size; ++n) {
          const double x(n - 0.5 * num_filter_order);
          h[n] = std::sin(omega * x) / (sptk::kPi * x);
        }
        // Avoid NaN using L'Hopital's rule.
        if (!sptk::IsEven(filter_size))
          h[num_filter_order / 2] = omega / sptk::kPi;
      }

      // Make prototype filter.
      std::transform(ideal_filter.begin(), ideal_filter.end(), window.begin(),
                     prototype_filter.begin(),
                     [](double h, double w) { return h * w; });

      // Get frequency response of the prototype filter.
      if (!fft.Run(prototype_filter, &real, &imag, &buffer_for_fft)) {
        return false;
      }

      // Calculate error.
      const double error(real[index] * real[index] + imag[index] * imag[index] -
                         0.5);
      const double abs_error(std::abs(error));
      if (abs_error < convergence_threshold) {
        if (is_converged) *is_converged = true;
        break;
      }

      // Update angular frequency.
      const int sign((0 < error) ? -1 : 1);
      if (abs_error < best_abs_error) {
        omega += sign * step_size;
        best_abs_error = abs_error;
      } else {
        omega += sign * step_size;
        step_size *= 0.5;  // Drop step size by half (heuristic).
        omega -= sign * step_size;
      }
    }
  }

  // Make filter banks.
  {
    filter_banks->resize(num_subband);
    int sign(inverse ? -1 : 1);
    for (int k(0); k < num_subband; ++k) {
      (*filter_banks)[k].resize(filter_size);
      double* p(&(prototype_filter[0]));
      double* h(&((*filter_banks)[k][0]));
      for (int n(0); n < filter_size; ++n) {
        const double a((2 * k + 1) * sptk::kPi / (2 * num_subband) *
                       (n - 0.5 * num_filter_order));
        const double b(sign * 0.25 * sptk::kPi);
        const double c(2.0 * p[n]);
        h[n] = c * std::cos(a + b);
      }
      sign *= -1;
    }
  }
  return true;
}

bool Perform1DConvolution(const std::vector<double>& f,
                          const std::vector<double>& g,
                          std::vector<double>* result) {
  if (f.empty() || g.empty() || NULL == result) {
    return false;
  }

  const int m(f.size());
  const int n(g.size());
  const int output_size(m + n - 1);
  if (result->size() != static_cast<std::size_t>(output_size)) {
    result->resize(output_size);
  }

  std::fill(result->begin(), result->end(), 0.0);
  double* output(&((*result)[0]));
  for (int i(0); i < output_size; ++i) {
    const int begin(std::max(i - n + 1, 0));
    const int end(std::min(i, m - 1));
    for (int j(begin); j <= end; ++j) {
      output[i] += f[j] * g[i - j];
    }
  }
  return true;
}

}  // namespace sptk
