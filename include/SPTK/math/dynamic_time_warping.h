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
//                1996-2020  Nagoya Institute of Technology          //
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

/**
 * Perform dynamic time warping.
 *
 * The input is the @f$M@f$-th order query vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}(1), & \boldsymbol{x}(2), & \ldots, & \boldsymbol{x}(T_x),
 *   \end{array}
 * @f]
 * and the @f$M@f$-th order reference vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{y}(1), & \boldsymbol{y}(2), & \ldots, & \boldsymbol{y}(T_y),
 *   \end{array}
 * @f]
 * where @f$T_x@f$ and @f$T_y@f$ are the length of the each vectors.
 * The output is the concatenated vector sequence:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{z}(1), & \boldsymbol{z}(2), & \ldots, & \boldsymbol{z}(T),
 *   \end{array}
 * @f]
 * where
 * @f[
 *   \boldsymbol{z}(t) = \left[ \begin{array}{c}
 *     \boldsymbol{x}(\phi_x(t)) \\ \boldsymbol{y}(\phi_y(t))
 *   \end{array} \right],
 * @f]
 * and @f$\phi_x(\cdot)@f$ and @f$\phi_y(\cdot)@f$ are the function which maps
 * the Viterbi time index into the corresponding time index of query/reference
 * data sequence, respectively.
 */
class DynamicTimeWarping {
 public:
  /**
   * Local path constraints.
   */
  enum LocalPathConstraints {
    /** @image html dtw_p0.png */
    kType0 = 0,

    /** @image html dtw_p1.png */
    kType1,

    /**
     * @image html dtw_p2.png
     *
     * Require @f$T_x >= T_y@f$.
     */
    kType2,

    /** @image html dtw_p3.png */
    kType3,

    /** @image html dtw_p4.png */
    kType4,

    /** @image html dtw_p5.png */
    kType5,

    /** @image html dtw_p6.png */
    kType6,

    kNumTypes
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] local_path_constraint Type of local path constraint.
   * @param[in] distance_metric Distance metric.
   */
  DynamicTimeWarping(int num_order, LocalPathConstraints local_path_constraint,
                     DistanceCalculation::DistanceMetrics distance_metric);

  virtual ~DynamicTimeWarping() {
  }

  /**
   * @return Order of vector.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Type of local path constraint.
   */
  LocalPathConstraints GetLocalPathConstraint() const {
    return local_path_constraint_;
  }

  /**
   * @return Distance metric.
   */
  DistanceCalculation::DistanceMetrics GetDistanceMetric() const {
    return distance_calculation_.GetDistanceMetric();
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] query_vector_sequence @f$M@f$-th order query vectors.
   *            The shape is @f$[T_x, M+1]@f$.
   * @param[in] reference_vector_sequence @f$M@f$-th order reference vectors.
   *            The shape is @f$[T_y, M+1]@f$.
   * @param[out] viterbi_path Best sequence of the pairs of index.
   * @param[out] total_score Score of dynamic time warping.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<std::vector<double> >& query_vector_sequence,
           const std::vector<std::vector<double> >& reference_vector_sequence,
           std::vector<std::pair<int, int> >* viterbi_path,
           double* total_score) const;

 private:
  const int num_order_;
  const LocalPathConstraints local_path_constraint_;
  const DistanceCalculation distance_calculation_;
  const bool includes_skip_transition_;

  bool is_valid_;

  std::vector<std::pair<int, int> > local_path_candidates_;
  std::vector<double> local_path_weights_;

  DISALLOW_COPY_AND_ASSIGN(DynamicTimeWarping);
};

}  // namespace sptk

#endif  // SPTK_MATH_DYNAMIC_TIME_WARPING_H_
