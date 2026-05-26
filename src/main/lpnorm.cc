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

#include <algorithm>  // std::transform
#include <fstream>    // std::ifstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "GETOPT/ya_getopt.h"
#include "SPTK/math/lp_norm_calculation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const double kDefaultNormOrder(2.0);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lpnorm - normalize by Lp-norm" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       lpnorm [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector   (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1 << "][   1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector    (   int)[" << std::setw(5) << std::right << "l-1"                << "][   0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -p p  : order of norm      (double)[" << std::setw(5) << std::right << kDefaultNormOrder    << "][ 1.0 <= p <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       vector                     (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       normalized vector          (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       for L-infinity norm, specify -p inf" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a lpnorm [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector
 * - @b -m @e int
 *   - order of vector
 * - @b -p @e double or @e str
 *   - order of norm
 * - @b infile @e str
 *   - double-type vector sequence
 * - @b stdout
 *   - double-type normalized vector sequence
 *
 * The below example calculates L2-normalized vector sequence:
 *
 * @code{.sh}
 *   echo 3 4 | x2x +ad | lpnorm -l 2 -p 2 | x2x +da
 *   # 0.6 0.8
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  double norm_order(kDefaultNormOrder);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:p:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("lpnorm", error_message);
          return 1;
        }
        --num_order;
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("lpnorm", error_message);
          return 1;
        }
        break;
      }
      case 'p': {
        if ((!sptk::ConvertSpecialStringToDouble(optarg, &norm_order) &&
             !sptk::ConvertStringToDouble(optarg, &norm_order)) ||
            norm_order < 1.0) {
          std::ostringstream error_message;
          error_message << "The argument for the -p option must be equal to or "
                        << "greater than 1.0";
          sptk::PrintErrorMessage("lpnorm", error_message);
          return 1;
        }
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

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("lpnorm", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("lpnorm", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("lpnorm", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  sptk::LpNormCalculation lp_norm_calculation(num_order, norm_order);
  if (!lp_norm_calculation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize LpNormCalculation";
    sptk::PrintErrorMessage("lpnorm", error_message);
    return 1;
  }

  const int vector_length(num_order + 1);
  std::vector<double> vector(vector_length);
  std::vector<double> normalized_vector(vector_length);

  while (sptk::ReadStream(false, 0, 0, vector_length, &vector, &input_stream,
                          NULL)) {
    double norm;
    if (!lp_norm_calculation.Run(vector, &norm)) {
      std::ostringstream error_message;
      error_message << "Failed to calculate Lp-norm";
      sptk::PrintErrorMessage("lpnorm", error_message);
      return 1;
    }

    std::transform(vector.begin(), vector.end(), normalized_vector.begin(),
                   [norm](double x) { return x / norm; });

    if (!sptk::WriteStream(0, vector_length, normalized_vector, &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write normalized vector";
      sptk::PrintErrorMessage("lpnorm", error_message);
      return 1;
    }
  }

  return 0;
}
