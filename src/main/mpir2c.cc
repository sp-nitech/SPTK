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

#include "Getopt/getoptwin.h"
#include "SPTK/conversion/minimum_phase_impulse_response_to_cepstrum.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumInputOrder(255);
const int kDefaultNumOutputOrder(25);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mpir2c - minimum phase impulse response to cepstrum" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mpir2c [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of minimum phase impulse response  (   int)[" << std::setw(5) << std::right << kDefaultNumInputOrder     << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -l l  : length of minimum phase impulse response (   int)[" << std::setw(5) << std::right << kDefaultNumInputOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -M M  : order of cepstrum                        (   int)[" << std::setw(5) << std::right << kDefaultNumOutputOrder    << "][ 0 <= M <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       minimum phase impulse response                   (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       cepstrum                                         (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a mpir2c [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of impulse response @f$(0 \le M_1)@f$
 * - @b -l @e int
 *   - length of impulse response @f$(1 \le M_1 + 1)@f$
 * - @b -M @e int
 *   - order of cesptral coefficients @f$(0 \le M_2)@f$
 * - @b infile @e str
 *   - double-type minimum phase impulse response
 * - @b stdout
 *   - double-type cepstral coefficients
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_input_order(kDefaultNumInputOrder);
  int num_output_order(kDefaultNumOutputOrder);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:l:M:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_input_order) ||
            num_input_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mpir2c", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_input_order) ||
            num_input_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("mpir2c", error_message);
          return 1;
        }
        --num_input_order;
        break;
      }
      case 'M': {
        if (!sptk::ConvertStringToInteger(optarg, &num_output_order) ||
            num_output_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -M option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mpir2c", error_message);
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
    sptk::PrintErrorMessage("mpir2c", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mpir2c", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::MinimumPhaseImpulseResponseToCepstrum
      minimum_phase_impulse_response_to_cepstrum(num_input_order,
                                                 num_output_order);
  if (!minimum_phase_impulse_response_to_cepstrum.IsValid()) {
    std::ostringstream error_message;
    error_message
        << "Failed to initialize MinimumPhaseImpulseResponseToCepstrum";
    sptk::PrintErrorMessage("mpir2c", error_message);
    return 1;
  }

  const int input_length(num_input_order + 1);
  const int output_length(num_output_order + 1);
  std::vector<double> minimum_phase_impulse_response(input_length);
  std::vector<double> cepstrum(output_length);

  while (sptk::ReadStream(false, 0, 0, input_length,
                          &minimum_phase_impulse_response, &input_stream,
                          NULL)) {
    if (!minimum_phase_impulse_response_to_cepstrum.Run(
            minimum_phase_impulse_response, &cepstrum)) {
      std::ostringstream error_message;
      error_message
          << "Failed to convert minimum phase impulse response to cepstrum";
      sptk::PrintErrorMessage("mpir2c", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, output_length, cepstrum, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write cepstrum";
      sptk::PrintErrorMessage("mpir2c", error_message);
      return 1;
    }
  }

  return 0;
}
