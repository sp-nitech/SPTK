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

#ifndef SPTK_COMPRESSION_LINDE_BUZO_GRAY_ALGORITHM_H_
#define SPTK_COMPRESSION_LINDE_BUZO_GRAY_ALGORITHM_H_

#include <vector>  // std::vector

#include "SPTK/math/distance_calculator.h"
#include "SPTK/math/statistics_accumulator.h"
#include "SPTK/compression/vector_quantization.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class LindeBuzoGrayAlgorithm {
 public:
  //
  LindeBuzoGrayAlgorithm(int num_order, int seed, int initial_codebook_size,
                         int target_codebook_size,
                         int minimum_num_vector_in_cluster, int num_iteration,
                         double convergence_threshold, double splitting_factor);

  //
  virtual ~LindeBuzoGrayAlgorithm() {
  }

  //
  int GetNumOrder() const {
    return num_order_;
  }

  //
  int GetSeed() const {
    return seed_;
  }

  //
  int GetInitialCodeBookSize() const {
    return initial_codebook_size_;
  }

  //
  int GetTargetCodeBookSize() const {
    return target_codebook_size_;
  }

  //
  int GetMinimumNumVectorInCluster() const {
    return minimum_num_vector_in_cluster_;
  }

  //
  int GetNumIteration() const {
    return num_iteration_;
  }

  //
  double GetConvergenceThreshold() const {
    return convergence_threshold_;
  }

  //
  double GetSplittingFactor() const {
    return splitting_factor_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(const std::vector<std::vector<double> >& input_vectors,
           std::vector<std::vector<double> >* codebook_vectors,
           std::vector<int>* codebook_index) const;

 private:
  //
  const int num_order_;

  //
  const int seed_;

  //
  const int initial_codebook_size_;

  //
  const int target_codebook_size_;

  //
  const int minimum_num_vector_in_cluster_;

  //
  const int num_iteration_;

  //
  const double convergence_threshold_;

  //
  const double splitting_factor_;

  //
  const DistanceCalculator distance_calculator_;

  //
  const StatisticsAccumulator statistics_accumulator_;

  //
  const VectorQuantization vector_quantization_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(LindeBuzoGrayAlgorithm);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_LINDE_BUZO_GRAY_ALGORITHM_H_
