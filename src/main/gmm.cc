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
const int kDefaultNumIteration(20);
const double kDefaultConvergenceThreshold(1e-5);
const double kDefaultWeightFloor(1e-5);
const double kDefaultVarianceFloor(1e-6);
const double kDefaultSmoothingParameter(0.0);
const bool kDefaultFullCovarianceFlag(false);
const bool kDefaultShowLikelihoodFlag(false);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " gmm - EM estimation of Gaussian mixture model" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       gmm [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector         (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1         << "][   1 <= l <=     ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector          (   int)[" << std::setw(5) << std::right << "l-1"                        << "][   0 <= m <=     ]" << std::endl;  // NOLINT
  *stream << "       -k k  : number of mixtures       (   int)[" << std::setw(5) << std::right << kDefaultNumMixture           << "][   1 <= k <=     ]" << std::endl;  // NOLINT
  *stream << "       -i i  : number of iterations     (   int)[" << std::setw(5) << std::right << kDefaultNumIteration         << "][   1 <= i <=     ]" << std::endl;  // NOLINT
  *stream << "       -d d  : convergence threshold    (double)[" << std::setw(5) << std::right << kDefaultConvergenceThreshold << "][ 0.0 <= d <=     ]" << std::endl;  // NOLINT
  *stream << "       -w w  : floor value of weight    (double)[" << std::setw(5) << std::right << kDefaultWeightFloor          << "][ 0.0 <= w <= 1/k ]" << std::endl;  // NOLINT
  *stream << "       -v v  : floor value of variance  (double)[" << std::setw(5) << std::right << kDefaultVarianceFloor        << "][ 0.0 <= v <=     ]" << std::endl;  // NOLINT
  *stream << "       -M M  : MAP smoothing parameter  (double)[" << std::setw(5) << std::right << kDefaultSmoothingParameter   << "][ 0.0 <= M <= 1.0 ]" << std::endl;  // NOLINT
  *stream << "       -U U  : input filename of double (string)[" << std::setw(5) << std::right << "N/A"                        << "]" << std::endl;  //  NOLINT
  *stream << "               type initial GMM parameters" << std::endl;
  *stream << "       -f    : use full covariance      (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultFullCovarianceFlag) << "]" << std::endl;  // NOLINT
  *stream << "       -V    : show log-likelihood      (  bool)[" << std::setw(5) << std::right << sptk::ConvertBooleanToString(kDefaultShowLikelihoodFlag) << "]" << std::endl;  // NOLINT
  *stream << "     (level 2)" << std::endl;
  *stream << "       -B B1 .. Bp : block size of      (   int)[" << std::setw(5) << std::right << "N/A"                        << "][   1 <= B <= l   ]" << std::endl;  // NOLINT
  *stream << "                     covariance matrix" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       training data sequence           (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       GMM parameters                   (double)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       -B option requires B1 + B2 + ... + Bp = l" << std::endl;
  *stream << "       -M option requires -U option" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a gmm [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le L - 1)@f$
 * - @b -k @e int
 *   - number of mixtures @f$(1 \le K)@f$
 * - @b -i @e int
 *   - number of iterations @f$(1 \le N)@f$
 * - @b -d @e double
 *   - convergence threshold @f$(0 \le \epsilon)@f$
 * - @b -w @e double
 *   - floor value of weight @f$(0 \le F_w \le 1/K)@f$
 * - @b -v @e double
 *   - floor value of variance @f$(0 \le F_v)@f$
 * - @b -M @e double
 *   - MAP smoothing parameter @f$(0 \le \alpha \le 1)@f$
 * - @b -U @e str
 *   - double-type initial GMM parameters
 * - @b -f @e bool
 *   - use full covariance
 * - @b -V @e bool
 *   - show log likelihood at each iteration
 * - @b -B @e int+
 *   - block size of covariance matrix
 * - @b infile @e str
 *   - double-type training data sequencea
 * - @b stdout
 *   - double-type GMM parameters
 *
 * The following examples show four types of covariance.
 *
 * @code{.sh}
 *   gmm -l 10 < data.d > diag.gmm
 *   gmm -l 10 -f < data.d > full.gmm
 *   gmm -l 10 -B 5 5 < data.d > block-wise-diag.gmm
 *   gmm -l 10 -f -B 5 5 < data.d > block-diag.gmm
 * @endcode
 *
 * If -M option is specified, the MAP estimates of the GMM paramaeters are
 * obtained using universal background model.
 *
 * @code{.sh}
 *   gmm -k 8 < data1.d > ubm.gmm
 *   gmm -k 8 -U ubm.gmm -M 0.1 < data2.d > map.gmm
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int num_mixture(kDefaultNumMixture);
  int num_iteration(kDefaultNumIteration);
  double convergence_threshold(kDefaultConvergenceThreshold);
  double weight_floor(kDefaultWeightFloor);
  double variance_floor(kDefaultVarianceFloor);
  double smoothing_parameter(kDefaultSmoothingParameter);
  const char* initial_gmm_file(NULL);
  bool full_covariance_flag(kDefaultFullCovarianceFlag);
  bool show_likelihood_flag(kDefaultShowLikelihoodFlag);
  std::vector<int> block_size;

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "l:m:k:i:d:w:v:M:U:fVB:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("gmm", error_message);
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
          sptk::PrintErrorMessage("gmm", error_message);
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
          sptk::PrintErrorMessage("gmm", error_message);
          return 1;
        }
        break;
      }
      case 'i': {
        if (!sptk::ConvertStringToInteger(optarg, &num_iteration) ||
            num_iteration <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -i option must be a positive integer";
          sptk::PrintErrorMessage("gmm", error_message);
          return 1;
        }
        break;
      }
      case 'd': {
        if (!sptk::ConvertStringToDouble(optarg, &convergence_threshold) ||
            convergence_threshold < 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -d option must be a non-negative number";
          sptk::PrintErrorMessage("gmm", error_message);
          return 1;
        }
        break;
      }
      case 'w': {
        if (!sptk::ConvertStringToDouble(optarg, &weight_floor) ||
            weight_floor < 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -w option must be a non-negative number";
          sptk::PrintErrorMessage("gmm", error_message);
          return 1;
        }
        break;
      }
      case 'v': {
        if (!sptk::ConvertStringToDouble(optarg, &variance_floor) ||
            variance_floor < 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -v option must be a non-negative number";
          sptk::PrintErrorMessage("gmm", error_message);
          return 1;
        }
        break;
      }
      case 'M': {
        if (!sptk::ConvertStringToDouble(optarg, &smoothing_parameter) ||
            smoothing_parameter < 0.0 || 1.0 < smoothing_parameter) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -v option must be in [0.0, 1.0]";
          sptk::PrintErrorMessage("gmm", error_message);
          return 1;
        }
        break;
      }
      case 'U': {
        initial_gmm_file = optarg;
        break;
      }
      case 'f': {
        full_covariance_flag = true;
        break;
      }
      case 'V': {
        show_likelihood_flag = true;
        break;
      }
      case 'B': {
        block_size.clear();
        int size;
        if (!sptk::ConvertStringToInteger(optarg, &size) || size <= 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -B option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("gmm", error_message);
          return 1;
        }
        block_size.push_back(size);
        while (optind < argc &&
               sptk::ConvertStringToInteger(argv[optind], &size)) {
          block_size.push_back(size);
          ++optind;
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

  if (block_size.empty()) {
    block_size.push_back(num_order + 1);
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("gmm", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::vector<std::vector<double> > input_vectors;
  {
    std::ifstream ifs;
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail() && NULL != input_file) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("gmm", error_message);
      return 1;
    }
    std::istream& input_stream(ifs.fail() ? std::cin : ifs);

    const int length(num_order + 1);
    std::vector<double> tmp(length);
    while (sptk::ReadStream(false, 0, 0, length, &tmp, &input_stream, NULL)) {
      input_vectors.push_back(tmp);
    }
  }
  if (input_vectors.empty()) return 0;

  const bool is_diagonal(!full_covariance_flag && 1 == block_size.size());

  std::vector<double> weights(num_mixture);
  std::vector<std::vector<double> > mean_vectors(num_mixture);
  std::vector<sptk::SymmetricMatrix> covariance_matrices(num_mixture);
  if (initial_gmm_file) {
    std::ifstream ifs;
    ifs.open(initial_gmm_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << initial_gmm_file;
      sptk::PrintErrorMessage("gmm", error_message);
      return 1;
    }
    std::istream& input_stream(ifs);

    for (int k(0); k < num_mixture; ++k) {
      if (!sptk::ReadStream(&(weights[k]), &input_stream)) {
        std::ostringstream error_message;
        error_message << "Failed to load mixture weight";
        sptk::PrintErrorMessage("gmm", error_message);
        return 1;
      }

      if (!sptk::ReadStream(false, 0, 0, num_order + 1, &mean_vectors[k],
                            &input_stream, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to load mean vector";
        sptk::PrintErrorMessage("gmm", error_message);
        return 1;
      }

      if (is_diagonal) {
        std::vector<double> variance;
        if (!sptk::ReadStream(false, 0, 0, num_order + 1, &variance,
                              &input_stream, NULL)) {
          std::ostringstream error_message;
          error_message << "Failed to load diagonal covariance vector";
          sptk::PrintErrorMessage("gmm", error_message);
          return 1;
        }
        covariance_matrices[k].Resize(num_order + 1);
        for (int l(0); l <= num_order; ++l) {
          covariance_matrices[k][l][l] = variance[l];
        }
      } else {
        covariance_matrices[k].Resize(num_order + 1);
        if (!sptk::ReadStream(&covariance_matrices[k], &input_stream)) {
          std::ostringstream error_message;
          error_message << "Failed to load covariance matrix";
          sptk::PrintErrorMessage("gmm", error_message);
          return 1;
        }
      }
    }
  }

  sptk::GaussianMixtureModeling gaussian_mixture_modeling(
      num_order, num_mixture, num_iteration, convergence_threshold,
      (full_covariance_flag
           ? sptk::GaussianMixtureModeling::CovarianceType::kFull
           : sptk::GaussianMixtureModeling::CovarianceType::kDiagonal),
      block_size, weight_floor, variance_floor,
      (initial_gmm_file
           ? sptk::GaussianMixtureModeling::InitializationType::kUbm
           : sptk::GaussianMixtureModeling::InitializationType::kKMeans),
      (show_likelihood_flag ? 1 : num_iteration + 1), smoothing_parameter,
      weights, mean_vectors, covariance_matrices);
  if (!gaussian_mixture_modeling.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize GaussianMixtureModel";
    sptk::PrintErrorMessage("gmm", error_message);
    return 1;
  }

  if (!gaussian_mixture_modeling.Run(input_vectors, &weights, &mean_vectors,
                                     &covariance_matrices)) {
    std::ostringstream error_message;
    error_message << "Failed to train Gaussian mixture models. "
                  << "Please consider the following attemps: "
                  << "a) increase training data; "
                  << "b) decrease number of mixtures; "
                  << "c) use (block) diagonal covariance";
    sptk::PrintErrorMessage("gmm", error_message);
    return 1;
  }

  for (int k(0); k < num_mixture; ++k) {
    if (!sptk::WriteStream(weights[k], &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write mixture weight";
      sptk::PrintErrorMessage("gmm", error_message);
      return 1;
    }

    if (!sptk::WriteStream(0, num_order + 1, mean_vectors[k], &std::cout,
                           NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write mean vector";
      sptk::PrintErrorMessage("gmm", error_message);
      return 1;
    }

    if (is_diagonal) {
      std::vector<double> variance;
      if (!covariance_matrices[k].GetDiagonal(&variance) ||
          !sptk::WriteStream(0, num_order + 1, variance, &std::cout, NULL)) {
        std::ostringstream error_message;
        error_message << "Failed to write diagonal covariance vector";
        sptk::PrintErrorMessage("gmm", error_message);
        return 1;
      }
    } else {
      if (!sptk::WriteStream(covariance_matrices[k], &std::cout)) {
        std::ostringstream error_message;
        error_message << "Failed to write covariance matrix";
        sptk::PrintErrorMessage("gmm", error_message);
        return 1;
      }
    }
  }

  return 0;
}
