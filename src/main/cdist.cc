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

#include <cmath>     // std::sqrt
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/math/distance_calculation.h"
#include "SPTK/math/statistics_accumulation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum OutputFormats {
  kEuclideanInDecibel = 0,
  kEuclidean,
  kSquaredEuclidean,
  kNumOutputFormats
};

const int kDefaultNumOrder(25);
const OutputFormats kDefaultOutputFormat(kEuclideanInDecibel);
const bool kDefaultOutputFrameByFrameFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " cdist - calculate cepstral distance" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       cdist [ options ] cfile [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of cepstrum     (   int)[" << std::setw(5) << std::right << kDefaultNumOrder     << "][ 1 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format         (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat << "][ 0 <= o <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (Euclidean [dB])" << std::endl;
  *stream << "                 1 (Euclidean)" << std::endl;
  *stream << "                 2 (squared Euclidean)" << std::endl;
  *stream << "       -f    : output frame by frame (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputFrameByFrameFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  cfile:" << std::endl;
  *stream << "       minimum-phase cepstrum        (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       minimum-phase cepstrum        (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       cepstral distance             (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       0th cepstral coefficient is ignored" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a cdist [ @e option ] @e cfile [ @e infile ]
 *
 * - @b -m @e int
 *   - order of coefficients @f$(0 \le M)@f$
 * - @b -o @e int
 *   - output format @f$(0 \le O \le 2)@f$
 *     \arg @c 0 Euclidean [dB]
 *     \arg @c 1 Euclidean
 *     \arg @c 2 squared Euclidean
 * - @b -f @e bool
 *   - output distance frame-by-frame
 * - @b cfile @e str
 *   - double-type minimum phase cepstrum
 * - @b infile @e str
 *   - double-type minimum phase cepstrum
 * - @b stdout
 *   - double-type cepstral distance
 *
 * The cepsturm model can be described as
 * @f[
 *   \log H(e^{j\omega}) = \sum_{m=0}^M c(m) e^{-j\omega m}.
 * @f]
 * Let us define
 * @f[
 *   \log H_0(e^{j\omega}) = \sum_{m=1}^M c(m) e^{-j\omega m},
 * @f]
 * where
 * @f[
 *   \log H_0(e^{j\omega}) = \log|H_0(e^{j\omega})| + j \arg H_0(e^{j\omega}).
 * @f]
 * Using the properties of complex conjugate, we obtain
 * @f{eqnarray}{
 *   \sum_{m=1}^M c(m) e^{-j\omega m} + \sum_{m=1}^M c(m) e^{j\omega m}
 *     &=& 2\log |H_0(e^{j\omega})| \\
 *   \sum_{m=-M}^M \hat{c}(m) e^{-j\omega m}
 *     &=& \log |H_0(e^{j\omega})|^2 \\
 * @f}
 * where
 * @f[
 *   \hat{c}(m) = \left\{ \begin{array}{ll}
 *     c(-m), & m < 0 \\
 *     0,     & m = 0 \\
 *     c(m).  & m > 0
 *   \end{array} \right.
 * @f]
 * The Parseval's theorem says that the distance between two cepstral
 * coefficients equivalents to the distance between two log spectra:
 * @f[
 *   \sum_{m=-M}^M (\hat{c}^{(1)}(m) - \hat{c}^{(2)}(m))^2
 *     = \frac{1}{2\pi} \int_{-\pi}^{\pi}
 *       \left( \log|H_0^{(1)}(e^{j\omega})|^2 - \log|H_0^{(2)}(e^{j\omega})|^2
 *       \right)^2 d\omega.
 * @f]
 * The Euclidean distance can be written as
 * @f[
 *   \sqrt{\sum_{m=-M}^M (\hat{c}^{(1)}(m) - \hat{c}^{(2)}(m))^2}
 *   = \sqrt{2 \sum_{m=1}^M (c^{(1)}(m) - c^{(2)}(m))^2}.
 * @f]
 * It can be converted into decibels by multipling a constant:
 * @f[
 *   \underbrace{\frac{10 \sqrt{2}}{\log 10}}_C
 *     \sqrt{\underbrace{\sum_{m=1}^M (c^{(1)}(m) - c^{(2)}(m))^2}_d}.
 * @f]
 *
 * The input of this command is
 * @f[
 *   \begin{array}{ccccc}
 *     c^{(1)}_t(0), & c^{(1)}_t(1), & \ldots, & c^{(1)}_t(M), & \ldots \\
 *     c^{(2)}_t(0), & c^{(2)}_t(1), & \ldots, & c^{(2)}_t(M), & \ldots
 *   \end{array}
 * @f]
 * and the output is
 * @f[
 *   \begin{array}{cccc}
 *     d'_t, & d'_{t+1}, & d'_{t+2}, & \ldots
 *   \end{array}
 * @f]
 * where
 * @f[
 *   d'_t = \left\{ \begin{array}{cl}
 *     C \sqrt{d_t}, & (O = 0) \\
 *     \sqrt{d_t}, & (O = 1) \\
 *     d_t. & (O = 2)
 *   \end{array} \right.
 * @f]
 * If @c -f option is not given, the output is the total cepstral distance
 * between two cepstral coefficients:
 * @f[
 *    d' = \frac{1}{T} \sum_{t=0}^{T-1} d'_t
 * @f]
 * where @f$T@f$ is the number of frames.
 *
 * In the example below, the spectral distance of the 15-th order cepstrum files
 * @c data1.cep and @c data2.cep is evaluated and displayed:
 *
 * @code{.sh}
 *   cdist -m 15 data1.cep data2.cep | dmp +d
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  OutputFormats output_format(kDefaultOutputFormat);
  bool output_frame_by_frame(kDefaultOutputFrameByFrameFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:o:fh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -m option must be a positive integer";
          sptk::PrintErrorMessage("cdist", error_message);
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
          sptk::PrintErrorMessage("cdist", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'f': {
        output_frame_by_frame = true;
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

  const char* cepstrum1_file;
  const char* cepstrum2_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    cepstrum1_file = argv[argc - 2];
    cepstrum2_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    cepstrum1_file = argv[argc - 1];
    cepstrum2_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, cfile and infile, are required";
    sptk::PrintErrorMessage("cdist", error_message);
    return 1;
  }

  std::ifstream ifs1;
  ifs1.open(cepstrum1_file, std::ios::in | std::ios::binary);
  if (ifs1.fail()) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << cepstrum1_file;
    sptk::PrintErrorMessage("cdist", error_message);
    return 1;
  }
  std::istream& stream_for_cepstrum1(ifs1);

  std::ifstream ifs2;
  ifs2.open(cepstrum2_file, std::ios::in | std::ios::binary);
  if (ifs2.fail() && NULL != cepstrum2_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << cepstrum2_file;
    sptk::PrintErrorMessage("cdist", error_message);
    return 1;
  }
  std::istream& stream_for_cepstrum2(ifs2.fail() ? std::cin : ifs2);

  sptk::StatisticsAccumulation statistics_accumulation(0, 1);
  sptk::StatisticsAccumulation::Buffer buffer;
  sptk::DistanceCalculation distance_calculation(
      num_order - 1,
      sptk::DistanceCalculation::DistanceMetrics::kSquaredEuclidean);
  if (!statistics_accumulation.IsValid() || !distance_calculation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize";
    sptk::PrintErrorMessage("cdist", error_message);
    return 1;
  }

  std::vector<double> cepstrum1(num_order);
  std::vector<double> cepstrum2(num_order);

  while (sptk::ReadStream(false, 1, 0, num_order, &cepstrum1,
                          &stream_for_cepstrum1, NULL) &&
         sptk::ReadStream(false, 1, 0, num_order, &cepstrum2,
                          &stream_for_cepstrum2, NULL)) {
    double distance;
    if (!distance_calculation.Run(cepstrum1, cepstrum2, &distance)) {
      std::ostringstream error_message;
      error_message << "Failed to calculate distance";
      sptk::PrintErrorMessage("cdist", error_message);
      return 1;
    }

    switch (output_format) {
      case kEuclideanInDecibel: {
        distance = 0.5 * sptk::kNeper * std::sqrt(2.0 * distance);
        break;
      }
      case kEuclidean: {
        distance = std::sqrt(distance);
        break;
      }
      case kSquaredEuclidean: {
        // nothing to do
        break;
      }
      default: {
        break;
      }
    }

    if (output_frame_by_frame) {
      if (!sptk::WriteStream(distance, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write distance";
        sptk::PrintErrorMessage("cdist", error_message);
        return 1;
      }
    } else {
      if (!statistics_accumulation.Run(std::vector<double>{distance},
                                       &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to accumulate statistics";
        sptk::PrintErrorMessage("cdist", error_message);
        return 1;
      }
    }
  }

  int num_data;
  if (!statistics_accumulation.GetNumData(buffer, &num_data)) {
    std::ostringstream error_message;
    error_message << "Failed to accumulate statistics";
    sptk::PrintErrorMessage("cdist", error_message);
    return 1;
  }

  if (!output_frame_by_frame && 0 < num_data) {
    std::vector<double> average_distance(1);
    if (!statistics_accumulation.GetMean(buffer, &average_distance)) {
      std::ostringstream error_message;
      error_message << "Failed to calculate distance";
      sptk::PrintErrorMessage("cdist", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, 1, average_distance, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write distance";
      sptk::PrintErrorMessage("cdist", error_message);
      return 1;
    }
  }

  return 0;
}
