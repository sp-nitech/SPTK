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
#include "SPTK/math/gaussian_mixture_modeling.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const int kDefaultNumMixture(16);
const bool kDefaultFullCovarianceFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " gmmp - compute log-probability of data using GMM" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       gmmp [ options ] gmmfile [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector    (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1 << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector     (   int)[" << std::setw(5) << std::right << "l-1"                << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -k k  : number of mixtures  (   int)[" << std::setw(5) << std::right << kDefaultNumMixture   << "][ 1 <= k <=   ]" << std::endl;  // NOLINT
  *stream << "       -f    : use full covariance (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultFullCovarianceFlag) << "]" << std::endl;  // NOLINT
  *stream << "               or block covariance" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  gmmfile:" << std::endl;
  *stream << "       GMM parameters              (double)" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       input data sequence         (double)[stdin]" << std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       log-probability sequence    (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       -B option requires B1 + B2 + ... + Bp = l" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a gmmp [ @e option ] @e gmmfile [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le L - 1)@f$
 * - @b -k @e int
 *   - number of mixtures @f$(1 \le K)@f$
 * - @b -f @e bool
 *   - use full or block covariance instead of diagonal one
 * - @b gmmfile @e str
 *   - double-type GMM parameters
 * - @b infile @e str
 *   - double-type input data sequencea
 * - @b stdout
 *   - double-type log-probability
 *
 * The input of this command is
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}(0), & \boldsymbol{x}(1), & \ldots, & \boldsymbol{x}(T-1),
 *   \end{array}
 * @f]
 * where @f$\boldsymbol{x}(t)@f$ is @f$L@f$-dimensional vector. The output is a
 * sequence of log-probabilities of the input vectors:
 * @f[
 *   \begin{array}{ccc}
 *     \log p(\boldsymbol{x}(0)), & \ldots, & \log p(\boldsymbol{x}(T-1)),
 *   \end{array}
 * @f]
 * where
 * @f[
 *   p(\boldsymbol{x}(t)) = \sum_{k=0}^{K-1} w_k
 *     \mathcal{N}(\boldsymbol{x}(t) \, | \,
 *                 \boldsymbol{\mu}_k, \boldsymbol{\varSigma}_k),
 * @f]
 * where @f$w_k@f$, @f$\boldsymbol{\mu}_k@f$, and
 * @f$\boldsymbol{\varSigma}_k@f$, are the parameters of GMM.
 *
 * In the following example, the log-probabilities of input data read from
 * @c data.d based on 4-mixture GMM are calculetaed and then averaged.
 *
 * @code{.sh}
 *   gmmp -k 4 data.gmm < data.d > data.p
 *   vstat -o 1 data.p > data.p.avg
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int num_mixture(kDefaultNumMixture);
  bool full_covariance_flag(kDefaultFullCovarianceFlag);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:k:fh", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("gmmp", error_message);
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
          sptk::PrintErrorMessage("gmmp", error_message);
          return 1;
        }
        break;
      }
      case 'k': {
        if (!sptk::ConvertStringToInteger(optarg, &num_mixture) ||
            num_mixture <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -k option must be a positive integer";
          sptk::PrintErrorMessage("gmmp", error_message);
          return 1;
        }
        break;
      }
      case 'f': {
        full_covariance_flag = true;
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

  // Get input file names.
  const char* gmm_file;
  const char* input_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    gmm_file = argv[argc - 2];
    input_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    gmm_file = argv[argc - 1];
    input_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, gmmfile and infile, are required";
    sptk::PrintErrorMessage("gmmp", error_message);
    return 1;
  }

  const bool is_diagonal(!full_covariance_flag);

  std::vector<double> weights(num_mixture);
  std::vector<std::vector<double> > mean_vectors(num_mixture);
  std::vector<sptk::SymmetricMatrix> covariance_matrices(num_mixture);
  {
    std::ifstream ifs;
    ifs.open(gmm_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << gmm_file;
      sptk::PrintErrorMessage("gmmp", error_message);
      return 1;
    }
    std::istream& input_stream(ifs);

    for (int k(0); k < num_mixture; ++k) {
      if (!sptk::ReadStream(&(weights[k]), &input_stream)) {
        std::ostringstream error_message;
        error_message << "Failed to load mixture weight";
        sptk::PrintErrorMessage("gmmp", error_message);
        return 1;
      }

      if (!sptk::ReadStream(false, 0, 0, num_order + 1, &mean_vectors[k],
                            &input_stream, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to load mean vector";
        sptk::PrintErrorMessage("gmmp", error_message);
        return 1;
      }

      covariance_matrices[k].Resize(num_order + 1);
      if (is_diagonal) {
        std::vector<double> variance;
        if (!sptk::ReadStream(false, 0, 0, num_order + 1, &variance,
                              &input_stream, NULL)) {
          std::ostringstream error_message;
          error_message << "Failed to load diagonal covariance vector";
          sptk::PrintErrorMessage("gmmp", error_message);
          return 1;
        }
        covariance_matrices[k].SetDiagonal(variance);
      } else {
        if (!sptk::ReadStream(&covariance_matrices[k], &input_stream)) {
          std::ostringstream error_message;
          error_message << "Failed to load covariance matrix";
          sptk::PrintErrorMessage("gmmp", error_message);
          return 1;
        }
      }
    }
  }

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("gmmp", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const int length(num_order + 1);
  std::vector<double> input_vector(length);
  sptk::GaussianMixtureModeling::Buffer buffer;

  while (sptk::ReadStream(false, 0, 0, length, &input_vector, &input_stream,
                          NULL)) {
    double log_probability;
    if (!sptk::GaussianMixtureModeling::CalculateLogProbability(
            num_order, num_mixture, is_diagonal, true, input_vector, weights,
            mean_vectors, covariance_matrices, NULL, &log_probability,
            &buffer)) {
      std::ostringstream error_message;
      error_message << "Failed to compute log-probability";
      sptk::PrintErrorMessage("gmmp", error_message);
      return 1;
    }
    if (!sptk::WriteStream(log_probability, &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write log-probability";
      sptk::PrintErrorMessage("gmmp", error_message);
      return 1;
    }
  }

  return 0;
}
