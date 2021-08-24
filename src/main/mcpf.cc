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
//                1996-2021  Nagoya Institute of Technology          //
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

#include <getopt.h>  // getopt_long

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/utils/mel_cepstrum_postfiltering.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const int kDefaultImpulseResponseLength(1024);
const int kDefaultOnsetIndex(2);
const double kDefaultAlpha(0.35);
const double kDefaultBeta(0.1);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mcpf - postfilter for mel-cepstrum" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mcpf [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -m m  : order of mel-cepstrum      (   int)[" << std::setw(5) << std::right << kDefaultNumOrder              << "][    0 <= m <  l   ]" << std::endl;  // NOLINT
  *stream << "       -l l  : length of impulse response (   int)[" << std::setw(5) << std::right << kDefaultImpulseResponseLength << "][    2 <= l <=     ]" << std::endl;  // NOLINT
  *stream << "       -s s  : onset index                (   int)[" << std::setw(5) << std::right << kDefaultOnsetIndex            << "][    0 <= s <= m   ]" << std::endl;  // NOLINT
  *stream << "       -a a  : all-pass constant          (double)[" << std::setw(5) << std::right << kDefaultAlpha                 << "][ -1.0 <  a <  1.0 ]" << std::endl;  // NOLINT
  *stream << "       -b b  : intensity                  (double)[" << std::setw(5) << std::right << kDefaultBeta                  << "][      <= b <=     ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       mel-cepstrum                       (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       postfiltered mel-cepstrum          (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       value of l must be a power of 2" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a mcpf [ @e option ] [ @e infile ]
 *
 * - @b -m @e int
 *   - order of mel-cepstral coefficients @f$(0 \le M < L)@f$
 * - @b -l @e int
 *   - length of impulse response @f$(M < L)@f$
 * - @b -s @e int
 *   - onset index @f$(0 \le S \le M)@f$
 * - @b -a @e double
 *   - all-pass constant @f$(|\alpha| < 1)@f$
 * - @b -b @e double
 *   - intensity @f$(\beta)@f$
 * - @b infile @e str
 *   - double-type mel-cepstral coefficients
 * - @b stdout
 *   - double-type postfiltered mel-cepstral coefficients
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int impulse_response_length(kDefaultImpulseResponseLength);
  int onset_index(kDefaultOnsetIndex);
  double alpha(kDefaultAlpha);
  double beta(kDefaultBeta);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "m:l:s:a:b:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mcpf", error_message);
          return 1;
        }
        break;
      }
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &impulse_response_length)) {
          std::ostringstream error_message;
          error_message << "The argument for the -l option must be an integer";
          sptk::PrintErrorMessage("mcpf", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &onset_index) ||
            onset_index < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mcpf", error_message);
          return 1;
        }
        break;
      }
      case 'a': {
        if (!sptk::ConvertStringToDouble(optarg, &alpha) ||
            !sptk::IsValidAlpha(alpha)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -a option must be in (-1.0, 1.0)";
          sptk::PrintErrorMessage("mcpf", error_message);
          return 1;
        }
        break;
      }
      case 'b': {
        if (!sptk::ConvertStringToDouble(optarg, &beta)) {
          std::ostringstream error_message;
          error_message << "The argument for the -b option must be a number";
          sptk::PrintErrorMessage("mcpf", error_message);
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

  if (impulse_response_length <= num_order) {
    std::ostringstream error_message;
    error_message
        << "Order of mel-cepstrum must be less than length of impulse response";
    sptk::PrintErrorMessage("mcpf", error_message);
    return 1;
  }

  if (num_order < onset_index) {
    std::ostringstream error_message;
    error_message << "Order of mel-cepstrum must be greater than onset index";
    sptk::PrintErrorMessage("mcpf", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("mcpf", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mcpf", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::MelCepstrumPostfiltering mel_cepstrum_postfiltering(
      num_order, impulse_response_length, onset_index, alpha, beta);
  sptk::MelCepstrumPostfiltering::Buffer buffer;
  if (!mel_cepstrum_postfiltering.IsValid()) {
    std::ostringstream error_message;
    error_message << "FFT length must be a power of 2 and greater than 1";
    sptk::PrintErrorMessage("mcpf", error_message);
    return 1;
  }

  const int length(num_order + 1);
  std::vector<double> mel_cepstrum(length);

  while (sptk::ReadStream(false, 0, 0, length, &mel_cepstrum, &input_stream,
                          NULL)) {
    if (!mel_cepstrum_postfiltering.Run(&mel_cepstrum, &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to apply postfilter for mel-cepstrum";
      sptk::PrintErrorMessage("mcpf", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, length, mel_cepstrum, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write postfiltered mel-cepstrum";
      sptk::PrintErrorMessage("mcpf", error_message);
      return 1;
    }
  }

  return 0;
}
