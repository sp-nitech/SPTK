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

#include <getopt.h>  // getopt_long
#include <fstream>   // std::ifstream, std::ofstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream
#include <vector>    // std::vector

#include "SPTK/compressor/linde_buzo_gray_algorithm.h"
#include "SPTK/math/statistics_accumulator.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultNumOrder(25);
const int kDefaultSeed(1);
const int kDefaultTargetCodebookSize(256);
const int kDefaultMinimumNumVectorInCluster(1);
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
  *stream << "       -l l  : length of vector              (   int)[" << std::setw(5) << std::right << kDefaultNumOrder + 1              << "][   0 <  l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vector               (   int)[" << std::setw(5) << std::right << "l-1"                             << "][   0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -s s  : seed                          (   int)[" << std::setw(5) << std::right << kDefaultSeed                      << "][     <= s <=   ]" << std::endl;  // NOLINT
  *stream << "       -e e  : target codebook size          (   int)[" << std::setw(5) << std::right << kDefaultTargetCodebookSize        << "][   2 <= e <=   ]" << std::endl;  // NOLINT
  *stream << "       -C C  : input filename of double type (string)[" << std::setw(5) << std::right << "N/A"                             << "]" << std::endl;  // NOLINT
  *stream << "               initial codebook" << std::endl;
  *stream << "       -I I  : output filename of int type   (string)[" << std::setw(5) << std::right << "N/A"                             << "]" << std::endl;  // NOLINT
  *stream << "               codebook index" << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "     (level 2)" << std::endl;
  *stream << "       -n n  : minimum number of vectors in  (   int)[" << std::setw(5) << std::right << kDefaultMinimumNumVectorInCluster << "][   0 <  n <=   ]" << std::endl;  // NOLINT
  *stream << "               a cluster" << std::endl;
  *stream << "       -i i  : maximum number of iterations  (   int)[" << std::setw(5) << std::right << kDefaultNumIteration              << "][   0 <  i <=   ]" << std::endl;  // NOLINT
  *stream << "       -d d  : convergence threshold         (double)[" << std::setw(5) << std::right << kDefaultConvergenceThreshold      << "][ 0.0 <= d <=   ]" << std::endl;  // NOLINT
  *stream << "       -r r  : splitting factor              (double)[" << std::setw(5) << std::right << kDefaultSplittingFactor           << "][ 0.0 <  r <=   ]" << std::endl;  // NOLINT
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

int main(int argc, char* argv[]) {
  int num_order(kDefaultNumOrder);
  int seed(kDefaultSeed);
  int target_codebook_size(kDefaultTargetCodebookSize);
  const char* initial_codebook_file(NULL);
  const char* codebook_index_file(NULL);
  int minimum_num_vector_in_cluster(kDefaultMinimumNumVectorInCluster);
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
        if (!sptk::ConvertStringToInteger(optarg,
                                          &minimum_num_vector_in_cluster) ||
            minimum_num_vector_in_cluster <= 0) {
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

  // get input file
  const int num_input_files(argc - optind);
  if (1 < num_input_files) {
    std::ostringstream error_message;
    error_message << "Too many input files";
    sptk::PrintErrorMessage("lbg", error_message);
    return 1;
  }
  const char* input_file(0 == num_input_files ? NULL : argv[optind]);

  // open stream
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
    sptk::StatisticsAccumulator statistics_accumulator(num_order, 1);
    sptk::StatisticsAccumulator::Buffer buffer;
    for (std::vector<std::vector<double> >::iterator itr(input_vectors.begin());
         itr != input_vectors.end(); ++itr) {
      if (!statistics_accumulator.Run(*itr, &buffer)) {
        std::ostringstream error_message;
        error_message << "Failed to initialize codebook";
        sptk::PrintErrorMessage("lbg", error_message);
        return 1;
      }
    }

    std::vector<double> tmp(length);
    if (!statistics_accumulator.GetMean(buffer, &tmp)) {
      std::ostringstream error_message;
      error_message << "Failed to initialize codebook";
      sptk::PrintErrorMessage("lbg", error_message);
      return 1;
    }
    codebook_vectors.push_back(tmp);
  } else {
    std::ifstream ifs;
    ifs.open(initial_codebook_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << initial_codebook_file;
      sptk::PrintErrorMessage("lbg", error_message);
      return 1;
    }

    std::vector<double> tmp(length);
    while (sptk::ReadStream(false, 0, 0, length, &tmp, &ifs, NULL)) {
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

  sptk::LindeBuzoGrayAlgorithm codebook_designer(
      num_order, seed, codebook_vectors.size(), target_codebook_size,
      minimum_num_vector_in_cluster, num_iteration, convergence_threshold,
      splitting_factor);
  if (!codebook_designer.IsValid()) {
    std::ostringstream error_message;
    error_message << "Failed to set the condition for codebook design";
    sptk::PrintErrorMessage("lbg", error_message);
    return 1;
  }

  std::vector<int> codebook_index(input_vectors.size());
  if (!codebook_designer.Run(input_vectors, &codebook_vectors,
                             &codebook_index)) {
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
    if (!sptk::WriteStream(0, codebook_index.size(), codebook_index,
                           &output_stream, NULL)) {
      std::ostringstream error_message;
      error_message << "Failed to write codebook index";
      sptk::PrintErrorMessage("lbg", error_message);
      return 1;
    }
  }

  return 0;
}
