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
//                1996-2019  Nagoya Institute of Technology          //
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
#include <cfloat>    // DBL_MAX
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultOutputLength(256);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " linear_intpl - linear interpolation of data" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       linear_intpl [ options ] [ infile ] > stdout" << std::endl;  // NOLINT
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : output length             (   int)[" << std::setw(5) << std::right << kDefaultOutputLength << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : output order              (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : minimum of x-axis         (double)[" << std::setw(5) << std::right << "BOS"                << "][   <= s <  e ]" << std::endl;  // NOLINT
  *stream << "       -e e  : maximum of x-axis         (double)[" << std::setw(5) << std::right << "EOS"                << "][ s <  e <=   ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       2-dimensional data sequence       (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       linear interpolated data sequence (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       x-axis values in input data must be in ascending order" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int output_length(kDefaultOutputLength);
  double minimum_x(-DBL_MAX);
  double maximum_x(DBL_MAX);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:s:e:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("linear_intpl", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &output_length) ||
            output_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("linear_intpl", error_message);
          return 1;
        }
        ++output_length;
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &minimum_x)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be numeric";
          sptk::PrintErrorMessage("linear_intpl", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToDouble(optarg, &maximum_x)) {
          std::ostringstream error_message;
          error_message << "The argument for the -e option must be numeric";
          sptk::PrintErrorMessage("linear_intpl", error_message);
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

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("linear_intpl", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("linear_intpl", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  std::vector<double> data_x;
  std::vector<double> data_y;
  {
    double tmp_x;
    double tmp_y;
    while (sptk::ReadStream(&tmp_x, &input_stream) &&
           sptk::ReadStream(&tmp_y, &input_stream)) {
      data_x.push_back(tmp_x);
      data_y.push_back(tmp_y);
    }
  }

  // check x-axis values
  const int input_length(data_x.size());
  for (int i(1); i < input_length; ++i) {
    if (data_x[i] <= data_x[i - 1]) {
      std::ostringstream error_message;
      error_message << "x-axis values in input data must be in ascending order";
      sptk::PrintErrorMessage("linear_intpl", error_message);
      return 1;
    }
  }

  if (-DBL_MAX == minimum_x) {
    minimum_x = data_x.front();
  } else {
    if (minimum_x < data_x.front()) {
      std::ostringstream error_message;
      error_message << "Minimum value must be equal to or greater than "
                    << "minimum values of x-axis in input data";
      sptk::PrintErrorMessage("linear_intpl", error_message);
      return 1;
    }
    if (data_x.back() <= minimum_x) {
      std::ostringstream error_message;
      error_message << "Minimum value must be less than maximum values of "
                       "x-axis in input data";
      sptk::PrintErrorMessage("linear_intpl", error_message);
      return 1;
    }
  }
  if (DBL_MAX == maximum_x) {
    maximum_x = data_x.back();
  } else {
    if (data_x.back() < maximum_x) {
      std::ostringstream error_message;
      error_message << "Maximum value must be equal to or less than "
                    << "maximum values of x-axis in input data";
      sptk::PrintErrorMessage("linear_interpl", error_message);
      return 1;
    }
    if (maximum_x <= data_x.front()) {
      std::ostringstream error_message;
      error_message << "Maximum value must be greater than minimum values of "
                       "x-axis in input data";
      sptk::PrintErrorMessage("linear_intpl", error_message);
      return 1;
    }
  }

  int start_index(0);
  while (data_x[start_index + 1] < minimum_x) {
    ++start_index;
  }

  if (maximum_x <= minimum_x) {
    std::ostringstream error_message;
    error_message << "Maximum of x must be greater than minimum of x";
    sptk::PrintErrorMessage("linear_intpl", error_message);
    return 1;
  }

  if (1 == output_length) {
    const double inverse_diff_x(
        1.0 / (data_x[start_index] - data_x[start_index + 1]));
    const double diff_y(data_y[start_index] - data_y[start_index + 1]);
    const double intercept((data_x[start_index] * data_y[start_index + 1] -
                            data_x[start_index + 1] * data_y[start_index]) *
                           inverse_diff_x);
    const double gradient(diff_y * inverse_diff_x);
    const double output_y(gradient * minimum_x + intercept);
    if (!sptk::WriteStream(output_y, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write data";
      sptk::PrintErrorMessage("linear_intpl", error_message);
      return 1;
    }
  } else {
    int output_count(0);
    double output_x(minimum_x);
    const double interval((maximum_x - minimum_x) /
                          static_cast<double>(output_length - 1));
    for (int input_index(start_index); input_index + 1 < input_length;
         ++input_index) {
      if (output_x <= data_x[input_index + 1]) {
        const double inverse_diff_x(
            1.0 / (data_x[input_index] - data_x[input_index + 1]));
        const double diff_y(data_y[input_index] - data_y[input_index + 1]);
        const double intercept((data_x[input_index] * data_y[input_index + 1] -
                                data_x[input_index + 1] * data_y[input_index]) *
                               inverse_diff_x);
        const double gradient(diff_y * inverse_diff_x);
        while (output_count < output_length &&
               output_x <= data_x[input_index + 1]) {
          const double output_y(gradient * output_x + intercept);
          if (!sptk::WriteStream(output_y, &std::cout)) {
            std::ostringstream error_message;
            error_message << "Failed to write data";
            sptk::PrintErrorMessage("linear_intpl", error_message);
            return 1;
          }
          ++output_count;
          output_x = minimum_x + interval * output_count;
        }
      }
    }
  }
  return 0;
}
