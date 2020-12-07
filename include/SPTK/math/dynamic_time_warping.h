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

#ifndef SPTK_MATH_DYNAMIC_TIME_WARPING_H_
#define SPTK_MATH_DYNAMIC_TIME_WARPING_H_

#include <utility>  // std::pair
#include <vector>   // std::vector

#include "SPTK/math/distance_calculation.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class DynamicTimeWarping {
 public:
  //
  enum LocalPathConstraints {
    kType1 = 0,
    kType2,
    kType3,
    kType4,
    kType5,
    kType6,
    kType7,
    kNumTypes
  };

  //
  DynamicTimeWarping(int num_order, LocalPathConstraints local_path_constraint,
                     DistanceCalculation::DistanceMetrics distance_metric);

  //
  virtual ~DynamicTimeWarping() {
  }

  //
  int GetNumOrder() const {
    return num_order_;
  }

  //
  LocalPathConstraints GetLocalPathConstraint() const {
    return local_path_constraint_;
  }

  //
  DistanceCalculation::DistanceMetrics GetDistanceMetric() const {
    return distance_calculation_.GetDistanceMetric();
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(const std::vector<std::vector<double> >& query_vector_sequence,
           const std::vector<std::vector<double> >& reference_vector_sequence,
           std::vector<std::pair<int, int> >* viterbi_path,
           double* total_score) const;

 private:
  //
  const int num_order_;

  //
  const LocalPathConstraints local_path_constraint_;

  //
  const DistanceCalculation distance_calculation_;

  //
  bool is_valid_;

  //
  std::vector<std::pair<int, int> > local_path_candidates_;

  //
  std::vector<double> local_path_weights_;

  //
  DISALLOW_COPY_AND_ASSIGN(DynamicTimeWarping);
};

}  // namespace sptk

#endif  // SPTK_MATH_DYNAMIC_TIME_WARPING_H_
