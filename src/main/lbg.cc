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

#include <fstream>   // std::ifstream, std::ofstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "Getopt/getoptwin.h"
#include "SPTK/compression/linde_buzo_gray_algorithm.h"
#include "SPTK/math/statistics_accumulation.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const int kDefaultSeed(1);
const int kDefaultTargetCodebookSize(256);
const int kDefaultMinNumVectorInCluster(1);
const int kDefaultNumIteration(1000);
const double kDefaultConvergenceThreshold(1e-5);
const double kDefaultSplittingFactor(1e-5);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " lbg - Linde-Buzo-Gray algorithm for vector quantizer design" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       lbg [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector              (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1          << "][   1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector               (   int)[" << std::setw(5) << std::right << "l-1"                         << "][   0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : seed                          (   int)[" << std::setw(5) << std::right << kDefaultSeed                  << "][     <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -e e  : target codebook size          (   int)[" << std::setw(5) << std::right << kDefaultTargetCodebookSize    << "][   2 <= e <=   ]" << std::endl;  // NOLINT
  *stream << "       -C C  : input filename of double type (string)[" << std::setw(5) << std::right << "N/A"                         << "]" << std::endl;  // NOLINT
  *stream << "               initial codebook" << std::endl;
  *stream << "       -I I  : output filename of int type   (string)[" << std::setw(5) << std::right << "N/A"                         << "]" << std::endl;  // NOLINT
  *stream << "               codebook index" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "     (level 2)" << std::endl;
  *stream << "       -n n  : minimum number of vectors in  (   int)[" << std::setw(5) << std::right << kDefaultMinNumVectorInCluster << "][   1 <= n <=   ]" << std::endl;  // NOLINT
  *stream << "               a cluster" << std::endl;
  *stream << "       -i i  : maximum number of iterations  (   int)[" << std::setw(5) << std::right << kDefaultNumIteration          << "][   1 <= i <=   ]" << std::endl;  // NOLINT
  *stream << "       -d d  : convergence threshold         (double)[" << std::setw(5) << std::right << kDefaultConvergenceThreshold  << "][ 0.0 <= d <=   ]" << std::endl;  // NOLINT
  *stream << "       -r r  : splitting factor              (double)[" << std::setw(5) << std::right << kDefaultSplittingFactor       << "][ 0.0 <  r <=   ]" << std::endl;  // NOLINT
  *stream << "  infile:" << std::endl;
  *stream << "       vectors                               (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       codebook                              (double)" << std::endl;  // NOLINT
  *stream << "  notice:" << std::endl;
  *stream << "       number of input vectors must be equal to or greater than n * e" << std::endl;  // NOLINT
  *stream << "       final codebook size may not be e because codebook size is always doubled" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a lbg [ @e option ] [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le M + 1)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -s @e int
 *   - random seed
 * - @b -e @e int
 *   - target codebook size @f$(2 \le I_E)@f$
 * - @b -C @e str
 *   - double-type initial codebook
 * - @b -I @e str
 *   - int-type output codebook index
 * - @b -n @e int
 *   - minimum number of vectors in a cluster @f$(1 \le V)@f$
 * - @b -i @e int
 *   - maximum number of iterations @f$(1 \le N)@f$
 * - @b -d @e double
 *   - convergence threshold @f$(0 \le \varepsilon)@f$
 * - @b -r @e double
 *   - splitting factor @f$(0 < r)@f$
 * - @b infile @e str
 *   - double-type input vectors
 * - @b stdout
 *   - double-type codebook
 *
 * If -C option is not specified, the initial codebook is generated from the
 * whole collection of training data as follows:
 * @f[
 *   \boldsymbol{c}_0 = \frac{1}{T} \sum_{t=0}^{T-1} \boldsymbol{x}(t)
 * @f]
 * where the codebook size is one.
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int seed(kDefaultSeed);
  int target_codebook_size(kDefaultTargetCodebookSize);
  const char* initial_codebook_file(NULL);
  const char* codebook_index_file(NULL);
  int min_num_vector_in_cluster(kDefaultMinNumVectorInCluster);
  int num_iteration(kDefaultNumIteration);
  double convergence_threshold(kDefaultConvergenceThreshold);
  double splitting_factor(kDefaultSplittingFactor);

  for (;;) {
    const int option_char(
        getopt_long(argc, argv, "l:m:s:e:C:I:n:i:d:r:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &num_order) ||
            num_order <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("lbg", error_message);
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
          sptk::PrintErrorMessage("lbg", error_message);
          return 1;
        }
        break;
      }
      case 's': {
        if (!sptk::ConvertStringToInteger(optarg, &seed)) {
          std::ostringstream error_message;
          error_message << "The argument for the -s option must be an integer";
          sptk::PrintErrorMessage("lbg", error_message);
          return 1;
        }
        break;
      }
      case 'e': {
        if (!sptk::ConvertStringToInteger(optarg, &target_codebook_size) ||
            target_codebook_size <= 1) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -e option must be greater than 1";
          sptk::PrintErrorMessage("lbg", error_message);
          return 1;
        }
        break;
      }
      case 'C': {
        initial_codebook_file = optarg;
        break;
      }
      case 'I': {
        codebook_index_file = optarg;
        break;
      }
      case 'n': {
        if (!sptk::ConvertStringToInteger(optarg, &min_num_vector_in_cluster) ||
            min_num_vector_in_cluster <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -n option must be a positive integer";
          sptk::PrintErrorMessage("lbg", error_message);
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
          sptk::PrintErrorMessage("lbg", error_message);
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
          sptk::PrintErrorMessage("lbg", error_message);
          return 1;
        }
        break;
      }
      case 'r': {
        if (!sptk::ConvertStringToDouble(optarg, &splitting_factor) ||
            splitting_factor <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -r option must be a positive number";
          sptk::PrintErrorMessage("lbg", error_message);
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
    sptk::PrintErrorMessage("lbg", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("lbg", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  const int length(num_order + 1);
  std::vector<std::vector<double> > input_vectors;
  {
    std::vector<double> tmp(length);
    while (sptk::ReadStream(false, 0, 0, length, &tmp, &input_stream, NULL)) {
      input_vectors.push_back(tmp);
    }
  }
  if (input_vectors.empty()) return 0;

  std::vector<std::vector<double> > codebook_vectors;
  if (NULL == initial_codebook_file) {
    sptk::StatisticsAccumulation statistics_accumulation(num_order, 1);
    sptk::StatisticsAccumulation::Buffer buffer;
    for (std::vector<std::vector<double> >::iterator itr(input_vectors.begin());
         itr != input_vectors.end(); ++itr) {
      if (!statistics_accumulation.Run(*itr, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to initialize codebook";
        sptk::PrintErrorMessage("lbg", error_message);
        return 1;
      }
    }

    std::vector<double> mean(length);
    if (!statistics_accumulation.GetMean(buffer, &mean)) {
      std::ostringstream error_message;
      error_message << "Failed to initialize codebook";
      sptk::PrintErrorMessage("lbg", error_message);
      return 1;
    }
    codebook_vectors.push_back(mean);
  } else {
    std::ifstream ifs2;
    ifs2.open(initial_codebook_file, std::ios::in | std::ios::binary);
    if (ifs2.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << initial_codebook_file;
      sptk::PrintErrorMessage("lbg", error_message);
      return 1;
    }

    std::vector<double> tmp(length);
    while (sptk::ReadStream(false, 0, 0, length, &tmp, &ifs2, NULL)) {
      codebook_vectors.push_back(tmp);
    }
  }

  std::ofstream ofs;
  if (NULL != codebook_index_file) {
    ofs.open(codebook_index_file, std::ios::out | std::ios::binary);
    if (ofs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << codebook_index_file;
      sptk::PrintErrorMessage("lbg", error_message);
      return 1;
    }
  }
  std::ostream& output_stream(ofs);

  sptk::LindeBuzoGrayAlgorithm codebook_design(
      num_order, static_cast<int>(codebook_vectors.size()),
      target_codebook_size, min_num_vector_in_cluster, num_iteration,
      convergence_threshold, splitting_factor, seed);
  if (!codebook_design.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to initialize LindeBuzoGrayAlgorithm";
    sptk::PrintErrorMessage("lbg", error_message);
    return 1;
  }

  std::vector<int> codebook_indices(input_vectors.size());
  if (!codebook_design.Run(input_vectors, &codebook_vectors,
                           &codebook_indices)) {
    std::ostringstream error_message;
    error_message << "Failed to design codebook";
    sptk::PrintErrorMessage("lbg", error_message);
    return 1;
  }

  for (std::vector<std::vector<double> >::iterator itr(
           codebook_vectors.begin());
       itr != codebook_vectors.end(); ++itr) {
    if (!sptk::WriteStream(0, length, *itr, &std::cout, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write codebook vector";
      sptk::PrintErrorMessage("lbg", error_message);
      return 1;
    }
  }

  if (NULL != codebook_index_file) {
    if (!sptk::WriteStream(0, static_cast<int>(codebook_indices.size()),
                           codebook_indices, &output_stream, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write codebook index";
      sptk::PrintErrorMessage("lbg", error_message);
      return 1;
    }
  }

  return 0;
}
