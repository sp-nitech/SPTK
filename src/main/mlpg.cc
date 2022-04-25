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
#include "SPTK/generation/nonrecursive_maximum_likelihood_parameter_generation.h"
#include "SPTK/generation/recursive_maximum_likelihood_parameter_generation.h"
#include "SPTK/input/input_source_from_stream.h"
#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/misc_utils.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

enum LongOptions {
  kMagic = 1000,
};

enum InputFormats {
  kMeanAndVariance = 0,
  kMeanAndPrecision,
  kMeanTimesPrecisionAndPrecision,
  kNumInputFormats
};

enum Modes { kRecursive = 0, kNonrecursive, kNumModes };

const int kDefaultNumOrder(25);
const int kDefaultNumPastFrame(30);
const InputFormats kDefaultInputFormat(kMeanAndVariance);
const Modes kDefaultMode(kRecursive);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " mlpg - maximum-likelihood parameter generation" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       mlpg [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l          : length of vector        (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m          : order of vector         (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s          : number of past frames   (   int)[" << std::setw(5) << std::right << kDefaultNumPastFrame << "][ r <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -q q          : input format            (   int)[" << std::setw(5) << std::right << kDefaultInputFormat  << "][ 0 <= q <= 2 ]" << std::endl;  // NOLINT
  *stream << "                         0 (mean and variance)" << std::endl;
  *stream << "                         1 (mean and precision)" << std::endl;
  *stream << "                         2 (mean x precision and precision)" << std::endl;  // NOLINT
  *stream << "       -d d1 d2 ...  : delta coefficients      (double)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "       -D D          : filename of double type (string)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "                       delta coefficients" << std::endl;
  *stream << "       -r r1 (r2)    : width of regression     (   int)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "                       coefficients" << std::endl;
  *stream << "       -magic magic  : magic number            (double)[" << std::setw(5) << std::right << "N/A"                << "]" << std::endl;  // NOLINT
  *stream << "       -R            : mode                    (   int)[" << std::setw(5) << std::right << kDefaultMode         << "][ 0 <= R <= 1 ]" << std::endl;  // NOLINT
  *stream << "                         0 (recursive)" << std::endl;
  *stream << "                         1 (non-recursive)" << std::endl;
  *stream << "       -h            : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       mean and variance parameter sequence    (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       static parameter sequence               (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       -d and -D options can be given multiple times" << std::endl;  // NOLINT
  *stream << "       -s option is valid only with R=0" << std::endl;
  *stream << "       -magic option is not supported with R=0" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class InputSourcePreprocessing : public sptk::InputSourceInterface {
 public:
  InputSourcePreprocessing(InputFormats input_format,
                           sptk::InputSourceInterface* source)
      : input_format_(input_format),
        half_read_size_(source ? source->GetSize() / 2 : 0),
        source_(source),
        is_valid_(true) {
    if (NULL == source || !source->IsValid()) {
      is_valid_ = false;
      return;
    }
  }

  virtual ~InputSourcePreprocessing() {
  }

  virtual int GetSize() const {
    return source_ ? source_->GetSize() : 0;
  }

  virtual bool IsValid() const {
    return is_valid_;
  }

  virtual bool Get(std::vector<double>* buffer) {
    if (!is_valid_) {
      return false;
    }
    if (!source_->Get(buffer)) {
      return false;
    }
    switch (input_format_) {
      case kMeanAndVariance: {
        // Nothing to do.
        break;
      }
      case kMeanAndPrecision: {
        double* variance(&((*buffer)[half_read_size_]));
        for (int i(0); i < half_read_size_; ++i) {
          variance[i] = 1.0 / variance[i];
        }
        break;
      }
      case kMeanTimesPrecisionAndPrecision: {
        double* variance(&((*buffer)[half_read_size_]));
        for (int i(0); i < half_read_size_; ++i) {
          variance[i] = 1.0 / variance[i];
        }
        double* mean(&((*buffer)[0]));
        for (int i(0); i < half_read_size_; ++i) {
          mean[i] *= variance[i];
        }
        break;
      }
      default: {
        break;
      }
    }
    return true;
  }

 private:
  const InputFormats input_format_;
  const int half_read_size_;

  InputSourceInterface* source_;
  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(InputSourcePreprocessing);
};

}  // namespace

/**
 * @a mlpg [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le M + 1)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -s @e int
 *   - number of past frames @f$(0 \le S)@f$
 * - @b -q @e int
 *   - input format
 *     \arg @c 0 @f$\boldsymbol{\mu}@f$, @f$\boldsymbol{\varSigma}@f$
 *     \arg @c 1 @f$\boldsymbol{\mu}@f$, @f$\boldsymbol{\varSigma}^{-1}@f$
 *     \arg @c 2 @f$\boldsymbol{\mu\varSigma}^{-1}@f$,
 *               @f$\boldsymbol{\varSigma}^{-1}@f$
 * - @b -d @e double+
 *   - delta coefficients
 * - @b -D @e string
 *   - filename of double-type delta coefficients
 * - @b -r @e int+
 *   - width of 1st (and 2nd) regression coefficients
 * - @b -magic @e double
 *   - magic number
 * - @b -R @e int
 *   - mode
 *     \arg @c 0 recursive (Kalman filter)
 *     \arg @c 1 non-recursive (Cholesky decomposition)
 * - @b infile @e str
 *   - double-type mean and variance parameter sequence
 * - @b stdout
 *   - double-type static parameter sequence
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int num_past_frame(kDefaultNumPastFrame);
  InputFormats input_format(kDefaultInputFormat);
  std::vector<std::vector<double> > window_coefficients;
  bool is_regression_specified(false);
  double magic_number(0.0);
  bool is_magic_number_specified(false);
  Modes mode(kDefaultMode);

  const struct option long_options[] = {
      {"magic", required_argument, NULL, kMagic},
      {0, 0, 0, 0},
  };

  for (;;) {
    const int option_char(
        getopt_long_only(argc, argv, "l:m:s:q:d:D:r:R:h", long_options, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("mlpg", error_message);
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
          sptk::PrintErrorMessage("mlpg", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &num_past_frame) ||
            num_past_frame < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("mlpg", error_message);
          return 1;
        }
        break;
      }
      case 'q': {
        const int min(0);
        const int max(static_cast<int>(kNumInputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -q option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("mlpg", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
        break;
      }
      case 'd': {
        if (is_regression_specified) {
          std::ostringstream error_message;
          error_message
              << "-d and -r options cannot be specified at the same time";
          sptk::PrintErrorMessage("mlpg", error_message);
          return 1;
        }

        std::vector<double> coefficients;
        double coefficient;
        if (!sptk::ConvertStringToDouble(optarg, &coefficient)) {
          std::ostringstream error_message;
          error_message << "The argument for the -d option must be numeric";
          sptk::PrintErrorMessage("mlpg", error_message);
          return 1;
        }
        coefficients.push_back(coefficient);
        while (optind < argc &&
               sptk::ConvertStringToDouble(argv[optind], &coefficient)) {
          coefficients.push_back(coefficient);
          ++optind;
        }
        window_coefficients.push_back(coefficients);
        break;
      }
      case 'D': {
        if (is_regression_specified) {
          std::ostringstream error_message;
          error_message
              << "-D and -r options cannot be specified at the same time";
          sptk::PrintErrorMessage("mlpg", error_message);
          return 1;
        }

        std::ifstream ifs;
        ifs.open(optarg, std::ios::in | std::ios::binary);
        if (ifs.fail()) {
          std::ostringstream error_message;
          error_message << "Cannot open file " << optarg;
          sptk::PrintErrorMessage("mlpg", error_message);
          return 1;
        }
        std::vector<double> coefficients;
        double coefficient;
        while (sptk::ReadStream(&coefficient, &ifs)) {
          coefficients.push_back(coefficient);
        }
        window_coefficients.push_back(coefficients);
        break;
      }
      case 'r': {
        if (is_regression_specified) {
          std::ostringstream error_message;
          error_message << "-r option cannot be specified multiple times";
          sptk::PrintErrorMessage("mlpg", error_message);
          return 1;
        }

        int n;
        // Set first order coefficients.
        {
          std::vector<double> coefficients;
          if (!sptk::ConvertStringToInteger(optarg, &n) ||
              !sptk::ComputeFirstOrderRegressionCoefficients(n,
                                                             &coefficients)) {
            std::ostringstream error_message;
            error_message
                << "The argument for the -r option must be positive integer(s)";
            sptk::PrintErrorMessage("mlpg", error_message);
            return 1;
          }
          window_coefficients.push_back(coefficients);
        }

        // Set second order coefficients.
        if (optind < argc && sptk::ConvertStringToInteger(argv[optind], &n)) {
          std::vector<double> coefficients;
          if (!sptk::ComputeSecondOrderRegressionCoefficients(n,
                                                              &coefficients)) {
            std::ostringstream error_message;
            error_message
                << "The argument for the -r option must be positive integer(s)";
            sptk::PrintErrorMessage("mlpg", error_message);
            return 1;
          }
          window_coefficients.push_back(coefficients);
          ++optind;
        }
        is_regression_specified = true;
        break;
      }
      case kMagic: {
        if (!sptk::ConvertStringToDouble(optarg, &magic_number)) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -magic option must be a number";
          sptk::PrintErrorMessage("mlpg", error_message);
          return 1;
        }
        is_magic_number_specified = true;
        break;
      }
      case 'R': {
        const int min(0);
        const int max(static_cast<int>(kNumModes) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -R option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("mlpg", error_message);
          return 1;
        }
        mode = static_cast<Modes>(tmp);
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
    sptk::PrintErrorMessage("mlpg", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("mlpg", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const int static_size(num_order + 1);
  const int read_size(2 * static_size *
                      static_cast<int>(window_coefficients.size() + 1));
  sptk::InputSourceFromStream input_source(false, read_size, &input_stream);
  InputSourcePreprocessing preprocessed_source(input_format, &input_source);

  if (kRecursive == mode) {
    if (is_magic_number_specified) {
      std::ostringstream error_message;
      error_message << "Magic number is not supported on recursive mode";
      sptk::PrintErrorMessage("mlpg", error_message);
      return 1;
    }

    sptk::RecursiveMaximumLikelihoodParameterGeneration generation(
        num_order, num_past_frame, window_coefficients, &preprocessed_source);
    if (!generation.IsValid()) {
      std::ostringstream error_message;
      error_message << "Failed to initialize "
                       "RecursiveMaximumLikelihoodParameterGeneration";
      sptk::PrintErrorMessage("mlpg", error_message);
      return 1;
    }

    std::vector<double> smoothed_static_parameters(static_size);
    while (generation.Get(&smoothed_static_parameters)) {
      if (!sptk::WriteStream(0, static_size, smoothed_static_parameters,
                             &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write static parameters";
        sptk::PrintErrorMessage("mlpg", error_message);
        return 1;
      }
    }
  } else if (kNonrecursive == mode) {
    sptk::NonrecursiveMaximumLikelihoodParameterGeneration generation(
        num_order, window_coefficients, is_magic_number_specified,
        magic_number);
    if (!generation.IsValid()) {
      std::ostringstream error_message;
      error_message << "Failed to initialize "
                    << "NonrecursiveMaximumLikelihoodParameterGeneration";
      sptk::PrintErrorMessage("mlpg", error_message);
      return 1;
    }

    std::vector<std::vector<double> > mean_vectors;
    std::vector<std::vector<double> > variance_vectors;
    {
      const int size(input_source.GetSize() / 2);
      std::vector<double> tmp;
      while (input_source.Get(&tmp)) {
        mean_vectors.push_back(
            std::vector<double>(tmp.begin(), tmp.begin() + size));
        variance_vectors.push_back(
            std::vector<double>(tmp.begin() + size, tmp.end()));
      }
    }

    std::vector<std::vector<double> > smoothed_static_parameters;
    if (!generation.Run(mean_vectors, variance_vectors,
                        &smoothed_static_parameters)) {
      std::ostringstream error_message;
      error_message << "Failed to perform MLPG";
      sptk::PrintErrorMessage("mlpg", error_message);
      return 1;
    }

    const int sequence_length(
        static_cast<int>(smoothed_static_parameters.size()));
    for (int t(0); t < sequence_length; ++t) {
      if (!sptk::WriteStream(0, static_size, smoothed_static_parameters[t],
                             &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write static parameters";
        sptk::PrintErrorMessage("mlpg", error_message);
        return 1;
      }
    }
  }

  return 0;
}
