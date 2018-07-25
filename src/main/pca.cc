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
#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

#include "SPTK/math/principal_component_analysis.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultVectorLength(25);
const int kDefaultNumPrincipalComponent(2);
const int kDefaultNumIteration(10000);
const double kDefaultConvergenceThreshold(1e-6);

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
  *stream << "       -v v  : output filename of double type (string)[" << std::setw(5) << std::right << "N/A"                         << "]" << std::endl;  // NOLINT
  *stream << "               eigenvalues and proportions" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       data sequence                          (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       mean vector and eigenvectors           (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  int num_principal_component(kDefaultNumPrincipalComponent);
  int num_iteration(kDefaultNumIteration);
  double convergence_threshold(kDefaultConvergenceThreshold);
  const char* eigenvalues_file(NULL);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:n:i:d:v:h", NULL, NULL));
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

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  // open stream to output eigenvalues
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

  // prepare for principal component analysis
  sptk::PrincipalComponentAnalysis principal_component_analysis(
      vector_length - 1, num_iteration, convergence_threshold);
  sptk::PrincipalComponentAnalysis::Buffer buffer;
  if (!principal_component_analysis.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set condition for principal component analysis";
    sptk::PrintErrorMessage("pca", error_message);
    return 1;
  }

  // read input data
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
    error_message << "Failed to apply principal component analysis";
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

    const double sum(
        std::accumulate(eigenvalues.begin(), eigenvalues.end(), 0.0));
    std::vector<double> proportions(num_principal_component);
    std::transform(eigenvalues.begin(),
                   eigenvalues.begin() + num_principal_component,
                   proportions.begin(),
                   std::bind1st(std::multiplies<double>(), 1.0 / sum));
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
