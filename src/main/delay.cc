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

#include <getopt.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>

#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultStartIndex(0);
const bool kDefaultKeepSequenceLengthFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " delay - delay data sequence" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       delay [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -s s  : start index          (   int)[" << std::setw(5) << std::right << kDefaultStartIndex << "][   <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -k    : keep sequence length (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultKeepSequenceLengthFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       delayed data sequence        (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       if s < 0, advance data sequence" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int start_index(kDefaultStartIndex);
  bool keep_sequence_length_flag(kDefaultKeepSequenceLengthFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "s:kh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &start_index)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be an integer";
          sptk::PrintErrorMessage("delay", error_message);
          return 1;
        }
        break;
      }
      case 'k': {
        keep_sequence_length_flag = true;
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
    sptk::PrintErrorMessage("delay", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("delay", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  if (start_index <= 0) {
    double data;
    int num_zeros(-start_index);
    for (int i(0); i < -start_index; ++i) {
      if (!sptk::ReadStream(&data, &input_stream)) {
        num_zeros = i;
        break;
      }
    }

    while (sptk::ReadStream(&data, &input_stream)) {
      if (!sptk::WriteStream(data, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write data";
        sptk::PrintErrorMessage("delay", error_message);
        return 1;
      }
    }

    if (keep_sequence_length_flag) {
      for (int i(0); i < num_zeros; ++i) {
        if (!sptk::WriteStream(0.0, &std::cout)) {
          std::ostringstream error_message;
          error_message << "Failed to write data";
          sptk::PrintErrorMessage("delay", error_message);
          return 1;
        }
      }
    }
  } else {
    double data;
    std::queue<double> stored_data;
    for (int i(0); i < start_index; ++i) {
      if (sptk::ReadStream(&data, &input_stream)) {
        stored_data.push(data);
      } else if (keep_sequence_length_flag) {
        return 0;
      }
      if (!sptk::WriteStream(0.0, &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write data";
        sptk::PrintErrorMessage("delay", error_message);
        return 1;
      }
    }

    while (sptk::ReadStream(&data, &input_stream)) {
      if (!sptk::WriteStream(stored_data.front(), &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write data";
        sptk::PrintErrorMessage("delay", error_message);
        return 1;
      }
      stored_data.pop();
      stored_data.push(data);
    }

    if (!keep_sequence_length_flag) {
      while (!stored_data.empty()) {
        if (!sptk::WriteStream(stored_data.front(), &std::cout)) {
          std::ostringstream error_message;
          error_message << "Failed to write data";
          sptk::PrintErrorMessage("delay", error_message);
          return 1;
        }
        stored_data.pop();
      }
    }
  }

  return 0;
}
