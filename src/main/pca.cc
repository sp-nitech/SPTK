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
#include <fstream>    // std::ifstream, std::ofstream
#include <iomanip>    // std::setw
#include <iostream>   // std::cerr, std::cin, std::cout, std::endl, etc.
#include <numeric>    // std::accumulate
#include <sstream>    // std::ostringstream
#include <vector>     // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/math/principal_component_analysis.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultVectorLength(25);
const int kDefaultNumPrincipalComponent(2);
const int kDefaultNumIteration(10000);
const double kDefaultConvergenceThreshold(1e-6);
const sptk::PrincipalComponentAnalysis::CovarianceType kDefaultCovarianceType(
    sptk::PrincipalComponentAnalysis::CovarianceType::kSampleCovariance);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " pca - principal component analysis" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       pca [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector               (   int)[" << std::setw(5) << std::right << kDefaultVectorLength          << "][   1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector                (   int)[" << std::setw(5) << std::right << "l-1"                         << "][   0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -n n  : number of principal components (   int)[" << std::setw(5) << std::right << kDefaultNumPrincipalComponent << "][   1 <= n <= l ]" << std::endl;  // NOLINT
  *stream << "       -i i  : maximum number of iterations   (   int)[" << std::setw(5) << std::right << kDefaultNumIteration          << "][   1 <= i <=   ]" << std::endl;  // NOLINT
  *stream << "       -d d  : convergence threshold          (double)[" << std::setw(5) << std::right << kDefaultConvergenceThreshold  << "][ 0.0 <= d <=   ]" << std::endl;  // NOLINT
  *stream << "       -u u  : covariance type                (   int)[" << std::setw(5) << std::right << kDefaultCovarianceType        << "][   0 <= u <= 2 ]" << std::endl;  // NOLINT
  *stream << "                 0 (sample covariance)" << std::endl;
  *stream << "                 1 (unbiased covariance)" << std::endl;
  *stream << "                 2 (correlation)" << std::endl;
  *stream << "       -v v  : output filename of double type (string)[" << std::setw(5) << std::right << "N/A"                         << "]" << std::endl;  // NOLINT
  *stream << "               eigenvalues and proportions" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       vector sequence                        (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       mean vector and eigenvectors           (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a pca [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -n @e int
 *   - number of principal components @f$(1 \le N \le L)@f$
 * - @b -i @e int
 *   - number of iterations @f$(1 \le I)@f$
 * - @b -d @e double
 *   - convergence threshold @f$(0 \le \epsilon)@f$
 * - @b -u @e int
 *   - covariance type
 *     @arg @c 0 sample covariance
 *     @arg @c 1 unbiased covariance
 *     @arg @c 2 correlation
 * - @b -v @e str
 *   - double-type eigenvalues and proportions
 * - @b infile @e str
 *   - double-type vector sequence
 * - @b stdout
 *   - double-type mean vector and eigenvectors
 *
 * In the below example, principal component analysis is applied to the
 * three-dimensional training vectors contained in @c data.d.
 * The eigenvectors and the eigenvalues are written to @c eigvec.dat and
 * @c eigval.dat, repectively.
 *
 * @code{.sh}
 *   pca -l 3 -n 2 -v eigval.dat < data.d > eigvec.dat
 * @endcode
 *
 * The eigenvalues are sorted in descending order.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  int num_principal_component(kDefaultNumPrincipalComponent);
  int num_iteration(kDefaultNumIteration);
  double convergence_threshold(kDefaultConvergenceThreshold);
  sptk::PrincipalComponentAnalysis::CovarianceType covariance_type(
      kDefaultCovarianceType);
  const char* eigenvalues_file(NULL);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "l:m:n:i:d:u:v:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("pca", error_message);
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
          sptk::PrintErrorMessage("pca", error_message);
          return 1;
        }
        ++vector_length;
        break;
      }
      case 'n': {
        if (!sptk::ConvertStringToInteger(optarg, &num_principal_component) ||
            num_principal_component <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -n option must be a positive integer";
          sptk::PrintErrorMessage("pca", error_message);
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
          sptk::PrintErrorMessage("pca", error_message);
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
          sptk::PrintErrorMessage("pca", error_message);
          return 1;
        }
        break;
      }
      case 'u': {
        const int min(0);
        const int max(
            static_cast<int>(sptk::PrincipalComponentAnalysis::CovarianceType::
                                 kNumCovarianceTypes) -
            1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -u option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("pca", error_message);
          return 1;
        }
        covariance_type =
            static_cast<sptk::PrincipalComponentAnalysis::CovarianceType>(tmp);
        break;
      }
      case 'v': {
        eigenvalues_file = optarg;
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

  if (vector_length < num_principal_component) {
    std::ostringstream error_message;
    error_message << "Number of principal components must be equal to or "
                  << "less than length of input vector";
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }

  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // Open stream for reading inputs.
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // Open stream for writing eigenvalues.
  std::ofstream ofs;
  if (NULL != eigenvalues_file) {
    ofs.open(eigenvalues_file, std::ios::out | std::ios::binary);
    if (ofs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << eigenvalues_file;
      sptk::PrintErrorMessage("pca", error_message);
      return 1;
    }
  }
  std::ostream& output_stream(ofs);

  sptk::PrincipalComponentAnalysis principal_component_analysis(
      vector_length - 1, num_iteration, convergence_threshold, covariance_type);
  sptk::PrincipalComponentAnalysis::Buffer buffer;
  if (!principal_component_analysis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize PrincipalComponentAnalysis";
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }

  // Read input data.
  std::vector<std::vector<double> > input_vectors;
  {
    std::vector<double> tmp(vector_length);
    while (sptk::ReadStream(false, 0, 0, vector_length, &tmp, &input_stream,
                            NULL)) {
      input_vectors.push_back(tmp);
    }
  }
  if (input_vectors.empty()) return 0;

  std::vector<double> mean_vector(vector_length);
  std::vector<double> eigenvalues(vector_length);
  sptk::Matrix eigenvector_matrix(vector_length, vector_length);
  if (!principal_component_analysis.Run(input_vectors, &mean_vector,
                                        &eigenvalues, &eigenvector_matrix,
                                        &buffer)) {
    std::ostringstream error_message;
    error_message << "Failed to perform principal component analysis";
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }

  if (!sptk::WriteStream(0, vector_length, mean_vector, &std::cout, NULL)) {
    std::ostringstream error_message;
    error_message << "Failed to write mean vector";
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }

  sptk::Matrix eigenvector_submatrix;
  if (!eigenvector_matrix.GetSubmatrix(0, num_principal_component, 0,
                                       vector_length, &eigenvector_submatrix)) {
    std::ostringstream error_message;
    error_message << "Failed to get eigenvectors";
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }
  if (!sptk::WriteStream(eigenvector_submatrix, &std::cout)) {
    std::ostringstream error_message;
    error_message << "Failed to write eigenvectors";
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }

  if (NULL != eigenvalues_file) {
    if (!sptk::WriteStream(0, num_principal_component, eigenvalues,
                           &output_stream, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write eigenvalues";
      sptk::PrintErrorMessage("pca", error_message);
      return 1;
    }

    const double norm(
        1.0 / std::accumulate(eigenvalues.begin(), eigenvalues.end(), 0.0));
    std::vector<double> proportions(num_principal_component);
    std::transform(eigenvalues.begin(),
                   eigenvalues.begin() + num_principal_component,
                   proportions.begin(), [norm](double l) { return l * norm; });
    if (!sptk::WriteStream(0, num_principal_component, proportions,
                           &output_stream, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write proportions";
      sptk::PrintErrorMessage("pca", error_message);
      return 1;
    }
  }

  return 0;
}
