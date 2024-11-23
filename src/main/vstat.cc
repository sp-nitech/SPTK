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

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "GETOPT/ya_getopt.h"
#include "SPTK/math/statistics_accumulation.h"
#include "SPTK/math/symmetric_matrix.h"
#include "SPTK/utils/misc_utils.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kMeanAndCovariance = 0,
  kMean,
  kCovariance,
  kStandardDeviation,
  kCorrelation,
  kPrecision,
  kMeanAndLowerAndUpperBounds,
  kSufficientStatistics,
  kNumOutputFormats
};

const int kMagicNumberForEndOfFile(-1);
const int kDefaultVectorLength(1);
const double kDefaultConfidenceLevel(95.0);
const OutputFormats kDefaultOutputFormat(kMeanAndCovariance);
const bool kDefaultOutputOnlyDiagonalElementsFlag(false);
const bool kDefaultNumericallyStableFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " vstat - vector statistics calculation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       vstat [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector     (   int)[" << std::setw(5) << std::right << kDefaultVectorLength    << "][ 1 <= l <=     ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector      (   int)[" << std::setw(5) << std::right << "l-1"                   << "][ 0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -t t  : output interval      (   int)[" << std::setw(5) << std::right << "EOF"                   << "][ 1 <= t <=     ]" << std::endl;  // NOLINT
  *stream << "       -c c  : confidence level     (double)[" << std::setw(5) << std::right << kDefaultConfidenceLevel << "][ 0 <  c <  100 ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format        (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat    << "][ 0 <= o <= 7   ]" << std::endl;  // NOLINT
  *stream << "                 0 (mean and covariance)" << std::endl;
  *stream << "                 1 (mean)" << std::endl;
  *stream << "                 2 (covariance)" << std::endl;
  *stream << "                 3 (standard deviation)" << std::endl;
  *stream << "                 4 (correlation)" << std::endl;
  *stream << "                 5 (precision)" << std::endl;
  *stream << "                 6 (mean and lower/upper bounds)" << std::endl;
  *stream << "                 7 (sufficient statistics)" << std::endl;
  *stream << "       -s s  : statistics file      (string)[" << std::setw(5) << std::right << "N/A"                   << "]" << std::endl;  // NOLINT
  *stream << "       -d    : output only diagonal (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputOnlyDiagonalElementsFlag) << "]" << std::endl;  // NOLINT
  *stream << "               elements" << std::endl;
  *stream << "       -e    : use a numerically    (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultNumericallyStableFlag)          << "]" << std::endl;  // NOLINT
  *stream << "               stable algorithm" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       vectors                      (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       statistics                   (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       -d is valid only if o = 0, 2 or 7" << std::endl;
  *stream << "       -s can be specified multiple times" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

bool OutputStatistics(const sptk::StatisticsAccumulation& accumulation,
                      const sptk::StatisticsAccumulation::Buffer& buffer,
                      int vector_length, OutputFormats output_format,
                      double confidence_level,
                      bool outputs_only_diagonal_elements) {
  if (kMeanAndCovariance == output_format || kMean == output_format ||
      kMeanAndLowerAndUpperBounds == output_format) {
    std::vector<double> mean(vector_length);
    if (!accumulation.GetMean(buffer, &mean)) {
      return false;
    }
    if (!sptk::WriteStream(0, vector_length, mean, &std::cout, NULL)) {
      return false;
    }
  }

  if (kMeanAndCovariance == output_format || kCovariance == output_format) {
    if (outputs_only_diagonal_elements) {
      std::vector<double> variance(vector_length);
      if (!accumulation.GetDiagonalCovariance(buffer, &variance)) {
        return false;
      }
      if (!sptk::WriteStream(0, vector_length, variance, &std::cout, NULL)) {
        return false;
      }
    } else {
      sptk::SymmetricMatrix variance(vector_length);
      if (!accumulation.GetFullCovariance(buffer, &variance)) {
        return false;
      }
      if (!sptk::WriteStream(variance, &std::cout)) {
        return false;
      }
    }
  }

  if (kStandardDeviation == output_format) {
    std::vector<double> standard_deviation(vector_length);
    if (!accumulation.GetStandardDeviation(buffer, &standard_deviation)) {
      return false;
    }
    if (!sptk::WriteStream(0, vector_length, standard_deviation, &std::cout,
                           NULL)) {
      return false;
    }
  }

  if (kCorrelation == output_format) {
    sptk::SymmetricMatrix correlation(vector_length);
    if (!accumulation.GetCorrelation(buffer, &correlation)) {
      return false;
    }
    if (!sptk::WriteStream(correlation, &std::cout)) {
      return false;
    }
  }

  if (kPrecision == output_format) {
    sptk::SymmetricMatrix variance(vector_length);
    if (!accumulation.GetFullCovariance(buffer, &variance)) {
      return false;
    }

    sptk::SymmetricMatrix precision_matrix(vector_length);
    if (!variance.Invert(&precision_matrix)) {
      return false;
    }

    if (!sptk::WriteStream(precision_matrix, &std::cout)) {
      return false;
    }
  }

  if (kMeanAndLowerAndUpperBounds == output_format) {
    int num_data;
    if (!accumulation.GetNumData(buffer, &num_data)) {
      return false;
    }
    std::vector<double> mean(vector_length);
    if (!accumulation.GetMean(buffer, &mean)) {
      return false;
    }
    std::vector<double> variance(vector_length);
    if (!accumulation.GetDiagonalCovariance(buffer, &variance)) {
      return false;
    }

    std::vector<double> lower_bound(vector_length);
    std::vector<double> upper_bound(vector_length);
    if (!sptk::ComputeLowerAndUpperBounds(confidence_level, num_data, mean,
                                          variance, &lower_bound,
                                          &upper_bound)) {
      return false;
    }

    if (!sptk::WriteStream(0, vector_length, lower_bound, &std::cout, NULL)) {
      return false;
    }
    if (!sptk::WriteStream(0, vector_length, upper_bound, &std::cout, NULL)) {
      return false;
    }
  }

  if (kSufficientStatistics == output_format) {
    int num_data;
    if (!accumulation.GetNumData(buffer, &num_data)) {
      return false;
    }
    if (!sptk::WriteStream(static_cast<double>(num_data), &std::cout)) {
      return false;
    }

    std::vector<double> first;
    if (!accumulation.GetFirst(buffer, &first)) {
      return false;
    }
    if (!sptk::WriteStream(0, vector_length, first, &std::cout, NULL)) {
      return false;
    }

    sptk::SymmetricMatrix second;
    if (!accumulation.GetSecond(buffer, &second)) {
      return false;
    }
    if (outputs_only_diagonal_elements) {
      std::vector<double> tmp(vector_length);
      if (!second.GetDiagonal(&tmp)) {
        return false;
      }
      if (!sptk::WriteStream(0, vector_length, tmp, &std::cout, NULL)) {
        return false;
      }
    } else {
      if (!sptk::WriteStream(second, &std::cout)) {
        return false;
      }
    }
  }

  return true;
}

}  // namespace

/**
 * @a vstat [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le L - 1)@f$
 * - @b -t @e int
 *   - output interval @f$(1 \le T)@f$
 * - @b -c @e double
 *   - confidence level @f$(0 < C < 100)@f$
 * - @b -o @e int
 *   - output format
 *     \arg @c 0 mean and covariance
 *     \arg @c 1 mean
 *     \arg @c 2 covariance
 *     \arg @c 3 standard deviation
 *     \arg @c 4 correlation
 *     \arg @c 5 precision
 *     \arg @c 6 mean and lower/upper bounds
 *     \arg @c 7 sufficient statistics
 * - @b -s @e str
 *   - statistics file
 * - @b -d
 *   - output only diagonal elements
 * - @b -e
 *   - use a numerically stable algorithm
 * - @b infile @e str
 *   - double-type vectors
 * - @b stdout
 *   - double-type statistics
 *
 * The input of this command is
 * @f[
 *   \begin{array}{c}
 *     \underbrace{
 *       \underbrace{x_1(1), \; \ldots, \; x_1(L)}_L, \; \ldots, \;
 *       \underbrace{x_T(1), \; \ldots, \; x_T(L)}_L,
 *     }_{L \times T} \; \ldots,
 *   \end{array}
 * @f]
 * and the output format depends on @c -o option.
 *
 * If @f$O=1@f$,
 * @f[
 *   \begin{array}{ccc}
 *     \underbrace{\mu_{0}(1), \; \ldots, \; \mu_{0}(L)}_L, &
 *     \underbrace{\mu_{T}(1), \; \ldots, \; \mu_{T}(L)}_L, &
 *     \ldots,
 *   \end{array}
 * @f]
 * where
 * @f[
 *   \mu_t(l) = \frac{1}{T} \sum_{\tau=1}^T x_{t+\tau}(l).
 * @f]
 *
 * If @f$O=2@f$,
 * @f[
 *   \begin{array}{cc}
 *     \underbrace{\sigma^2_0(1,1),  \; \sigma^2_{0}(1,2), \; \ldots, \;
 *                 \sigma^2_0(L,L)}_{L \times L}, &
 *     \ldots,
 *   \end{array}
 * @f]
 * where
 * @f[
 *   \sigma^2_t(k,l) = \frac{1}{T} \sum_{\tau=1}^T
 *     (x_{t+\tau}(k) - \mu_t(k)) (x_{t+\tau}(l) - \mu_t(l)).
 * @f]
 *
 * If @f$O=3@f$,
 * @f[
 *   \begin{array}{cc}
 *     \underbrace{\sigma_{0}(1,1), \; \sigma_{0}(2,2), \; \ldots, \;
 *                 \sigma_{0}(L,L)}_L, &
 *     \ldots.
 *   \end{array}
 * @f]
 *
 * If @f$O=4@f$,
 * @f[
 *   \begin{array}{cc}
 *     \underbrace{r_{0}(1,1), \; r_{0}(1,2), \; \ldots, \;
 *                 r_{0}(L,L)}_{L \times L}, &
 *     \ldots,
 *   \end{array}
 * @f]
 * where
 * @f[
 *   r_t(k,l) = \frac{\sigma^2_t(k,l)}{\sigma_t(k,k) \, \sigma_t(l,l)}.
 * @f]
 *
 * If @f$O=5@f$,
 * @f[
 *   \begin{array}{cc}
 *     \underbrace{s_{0}(1,1), \; s_{0}(1,2), \; \ldots, \;
 *                 s_{0}(L,L)}_{L \times L}, &
 *     \ldots,
 *   \end{array}
 * @f]
 * where @f$s_t(k,l)@f$ is the @f$(k,l)@f$-th component of the inverse of the
 * covariance matrix.
 *
 * If @f$O=6@f$,
 * @f[
 *   \begin{array}{cccc}
 *     \underbrace{\mu_{0}(1), \; \ldots, \; \mu_{0}(L)}_L, &
 *     \underbrace{\ell_{0}(1), \; \ldots, \; \ell_{0}(L)}_L, &
 *     \underbrace{u_{0}(1), \; \ldots, \; u_{0}(L)}_L, &
 *     \ldots,
 *   \end{array}
 * @f]
 * where
 * @f{eqnarray}{
 *   \ell_t(l) &=& \mu_t(l) - p(C, T-1) \sqrt{\frac{\sigma^2_t(l,l)}{T-1}}, \\
 *      u_t(l) &=& \mu_t(l) + p(C, T-1) \sqrt{\frac{\sigma^2_t(l,l)}{T-1}},
 * @f}
 * and @f$p(C, T-1)@f$ is the upper @f$(100-C)/2@f$-th percentile of the
 * t-distribution with degrees of freedom @f$T-1@f$.
 *
 * If @f$O=7@f$,
 * @f[
 *   \begin{array}{cccc}
 *     \underbrace{T}_1, &
 *     \underbrace{m^{(1)}_{0}(1), \; \ldots, \; m^{(1)}_{0}(L)}_L, &
 *     \underbrace{m^{(2)}_{0}(1,1), \; m^{(2)}_{0}(1,2), \; \ldots, \;
 *                 m^{(2)}_{0}(L,L)}_{L \times L}, &
 *     \ldots,
 *   \end{array}
 * @f]
 * where @f$m^{(1)}@f$ is the first-order statistics and @f$m^{(2)}@f$ is the
 * second-order statistics.
 *
 * @code{.sh}
 *   echo 0 1 2 3 4 5 6 7 8 9 | x2x +ad > data.d
 *   vstat -o 1 data.d | x2x +da
 *   # 4.5
 *   vstat -o 1 data.d -t 5 | x2x +da
 *   # 2, 7
 * @endcode
 *
 * @code{.sh}
 *   cat data1.d data2.d | vstat -o 7 > data12.stat
 *   cat data3.d data4.d | vstat -o 7 > data34.stat
 *   echo | vstat -s data12.stat -s data34.stat -o 1 > data.mean
 *   # equivalent to the following line
 *   cat data?.d | vstat -o 1 > data.mean
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  int output_interval(kMagicNumberForEndOfFile);
  double confidence_level(kDefaultConfidenceLevel);
  OutputFormats output_format(kDefaultOutputFormat);
  std::vector<const char*> statistics_file;
  bool outputs_only_diagonal_elements(kDefaultOutputOnlyDiagonalElementsFlag);
  bool numerically_stable(kDefaultNumericallyStableFlag);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "l:m:t:c:o:s:deh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("vstat", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("vstat", error_message);
          return 1;
        }
        ++vector_length;
        break;
      }
      case 't': {
        if (!sptk::ConvertStringToInteger(optarg, &output_interval) ||
            output_interval <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -t option must be a positive integer";
          sptk::PrintErrorMessage("vstat", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        if (!sptk::ConvertStringToDouble(optarg, &confidence_level) ||
            confidence_level <= 0.0 || 100.0 <= confidence_level) {
          std::ostringstream error_message;
          error_message << "The argument for the -c option must be a number "
                        << "in the open interval (0, 100)";
          sptk::PrintErrorMessage("vstat", error_message);
          return 1;
        }
        break;
      }
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(kNumOutputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("vstat", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 's': {
        statistics_file.push_back(optarg);
        break;
      }
      case 'd': {
        outputs_only_diagonal_elements = true;
        break;
      }
      case 'e': {
        numerically_stable = true;
        break;
      }
      case 'h': {
        PrintUsage(&std::cout);
        return 0;
      }
      default: {
        PrintUsage(&std::cerr);
        return 1;
      }
    }
  }

  if (kMagicNumberForEndOfFile != output_interval && !statistics_file.empty()) {
    std::ostringstream error_message;
    error_message << "Cannot specify -t option and -s option at the same time";
    sptk::PrintErrorMessage("vstat", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("vstat", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("vstat", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("vstat", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  bool diagonal(false);
  if (kMeanAndCovariance == output_format || kCovariance == output_format ||
      kSufficientStatistics == output_format) {
    if (outputs_only_diagonal_elements) {
      diagonal = true;
    }
  } else if (kStandardDeviation == output_format ||
             kMeanAndLowerAndUpperBounds == output_format) {
    diagonal = true;
  }

  sptk::StatisticsAccumulation accumulation(vector_length - 1,
                                            kMean == output_format ? 1 : 2,
                                            diagonal, numerically_stable);
  sptk::StatisticsAccumulation::Buffer buffer;
  if (!accumulation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize StatisticsAccumulation";
    sptk::PrintErrorMessage("vstat", error_message);
    return 1;
  }

  for (const char* file : statistics_file) {
    std::ifstream ifs2;
    ifs2.open(file, std::ios::in | std::ios::binary);
    if (ifs2.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << file;
      sptk::PrintErrorMessage("vstat", error_message);
      return 1;
    }

    double num_data;
    std::vector<double> first(vector_length);
    std::vector<double> tmp(vector_length);
    sptk::SymmetricMatrix second(vector_length);
    while (sptk::ReadStream(&num_data, &ifs2)) {
      if (!sptk::ReadStream(false, 0, 0, vector_length, &first, &ifs2, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to read statistics (first order) in " << file;
        sptk::PrintErrorMessage("vstat", error_message);
        return 1;
      }

      if (diagonal) {
        if (!sptk::ReadStream(false, 0, 0, vector_length, &tmp, &ifs2, NULL) ||
            !second.SetDiagonal(tmp)) {
          std::ostringstream error_message;
          error_message << "Failed to read statistics (second order) in "
                        << file << " (expected diagonal matrix)";
          sptk::PrintErrorMessage("vstat", error_message);
          return 1;
        }
      } else {
        if (!sptk::ReadStream(&second, &ifs2)) {
          std::ostringstream error_message;
          error_message << "Failed to read statistics (second order) in "
                        << file << " (expected full matrix)";
          sptk::PrintErrorMessage("vstat", error_message);
          return 1;
        }
      }

      if (!accumulation.Merge(static_cast<int>(num_data), first, second,
                              &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to merge statistics";
        sptk::PrintErrorMessage("vstat_merge", error_message);
        return 1;
      }
    }
  }

  std::vector<double> data(vector_length);
  for (int vector_index(1);
       sptk::ReadStream(false, 0, 0, vector_length, &data, &input_stream, NULL);
       ++vector_index) {
    if (!accumulation.Run(data, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to accumulate statistics";
      sptk::PrintErrorMessage("vstat", error_message);
      return 1;
    }

    if (kMagicNumberForEndOfFile != output_interval &&
        0 == vector_index % output_interval) {
      if (!OutputStatistics(accumulation, buffer, vector_length, output_format,
                            confidence_level, outputs_only_diagonal_elements)) {
        std::ostringstream error_message;
        error_message << "Failed to write statistics";
        sptk::PrintErrorMessage("vstat", error_message);
        return 1;
      }
      accumulation.Clear(&buffer);
    }
  }

  int num_data;
  if (!accumulation.GetNumData(buffer, &num_data)) {
    std::ostringstream error_message;
    error_message << "Failed to accumulate statistics";
    sptk::PrintErrorMessage("vstat", error_message);
    return 1;
  }

  if (kMagicNumberForEndOfFile == output_interval && 0 < num_data) {
    if (!OutputStatistics(accumulation, buffer, vector_length, output_format,
                          confidence_level, outputs_only_diagonal_elements)) {
      std::ostringstream error_message;
      error_message << "Failed to write statistics";
      sptk::PrintErrorMessage("vstat", error_message);
      return 1;
    }
  }

  return 0;
}
