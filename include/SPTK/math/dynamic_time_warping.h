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
