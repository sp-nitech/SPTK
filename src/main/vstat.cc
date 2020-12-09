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
#include <cmath>     // std::sqrt
#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

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
  kNumOutputFormats
};

const int kMagicNumberForEndOfFile(-1);
const int kDefaultVectorLength(1);
const double kDefaultConfidenceLevel(95.0);
const OutputFormats kDefaultOutputFormat(kMeanAndCovariance);
const bool kDefaultOutputOnlyDiagonalElementsFlag(false);

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
  *stream << "       -o o  : output format        (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat    << "][ 0 <= o <= 6   ]" << std::endl;  // NOLINT
  *stream << "                 0 (mean and covariance)" << std::endl;
  *stream << "                 1 (mean)" << std::endl;
  *stream << "                 2 (covariance)" << std::endl;
  *stream << "                 3 (standard deviation)" << std::endl;
  *stream << "                 4 (correlation)" << std::endl;
  *stream << "                 5 (precision)" << std::endl;
  *stream << "                 6 (mean and lower/upper bounds)" << std::endl;
  *stream << "       -d    : output only diagonal (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultOutputOnlyDiagonalElementsFlag) << "]" << std::endl;  // NOLINT
  *stream << "               elements" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       vectors                      (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       statistics                   (double)" << std::endl;
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
      for (int i(0); i < vector_length; ++i) {
        for (int j(0); j < vector_length; ++j) {
          if (!sptk::WriteStream(variance[i][j], &std::cout)) {
            return false;
          }
        }
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
    for (int i(0); i < vector_length; ++i) {
      for (int j(0); j < vector_length; ++j) {
        if (!sptk::WriteStream(correlation[i][j], &std::cout)) {
          return false;
        }
      }
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

    if (outputs_only_diagonal_elements) {
      for (int i(0); i < vector_length; ++i) {
        if (!sptk::WriteStream(precision_matrix[i][i], &std::cout)) {
          return false;
        }
      }
    } else {
      for (int i(0); i < vector_length; ++i) {
        for (int j(0); j < vector_length; ++j) {
          if (!sptk::WriteStream(precision_matrix[i][j], &std::cout)) {
            return false;
          }
        }
      }
    }
  }

  if (kMeanAndLowerAndUpperBounds == output_format) {
    int num_vector;
    if (!accumulation.GetNumData(buffer, &num_vector)) {
      return false;
    }

    const int degrees_of_freedom(num_vector - 1);
    if (0 == degrees_of_freedom) {
      return false;
    }
    double t;
    if (!sptk::ComputePercentagePointOfTDistribution(
            0.5 * (1.0 - confidence_level / 100.0), degrees_of_freedom, &t)) {
      return false;
    }
    std::vector<double> mean(vector_length);
    std::vector<double> variance(vector_length);
    if (!accumulation.GetMean(buffer, &mean)) {
      return false;
    }
    if (!accumulation.GetDiagonalCovariance(buffer, &variance)) {
      return false;
    }

    const double inverse_degrees_of_freedom(1.0 / degrees_of_freedom);
    std::vector<double> lower_bound(vector_length);
    std::vector<double> upper_bound(vector_length);
    for (int i(0); i < vector_length; ++i) {
      const double error(std::sqrt(variance[i] * inverse_degrees_of_freedom));
      lower_bound[i] = mean[i] - t * error;
      upper_bound[i] = mean[i] + t * error;
    }
    if (!sptk::WriteStream(0, vector_length, lower_bound, &std::cout, NULL)) {
      return false;
    }
    if (!sptk::WriteStream(0, vector_length, upper_bound, &std::cout, NULL)) {
      return false;
    }
  }

  return true;
}

}  // namespace

int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  int output_interval(kMagicNumberForEndOfFile);
  double confidence_level(kDefaultConfidenceLevel);
  OutputFormats output_format(kDefaultOutputFormat);
  bool outputs_only_diagonal_elements(kDefaultOutputOnlyDiagonalElementsFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:t:c:o:dh", NULL, NULL));
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
      case 'd': {
        outputs_only_diagonal_elements = true;
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
    sptk::PrintErrorMessage("vstat", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("vstat", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::StatisticsAccumulation accumulation(vector_length - 1, 2);
  sptk::StatisticsAccumulation::Buffer buffer;
  if (!accumulation.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for accumulation";
    sptk::PrintErrorMessage("vstat", error_message);
    return 1;
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

  int num_actual_vector;
  if (!accumulation.GetNumData(buffer, &num_actual_vector)) {
    std::ostringstream error_message;
    error_message << "Failed to accumulate statistics";
    sptk::PrintErrorMessage("vstat", error_message);
    return 1;
  }

  if (kMagicNumberForEndOfFile == output_interval && 0 < num_actual_vector) {
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
