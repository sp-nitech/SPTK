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

#ifndef SPTK_COMPRESSION_LINDE_BUZO_GRAY_ALGORITHM_H_
#define SPTK_COMPRESSION_LINDE_BUZO_GRAY_ALGORITHM_H_

#include <vector>  // std::vector

#include "SPTK/compression/vector_quantization.h"
#include "SPTK/math/distance_calculation.h"
#include "SPTK/math/statistics_accumulation.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Design codebook.
 *
 * The input is the @f$M@f$-th order input vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}_0, & \boldsymbol{x}_1, & \ldots, & \boldsymbol{x}_{T-1},
 *   \end{array}
 * @f]
 * where @f$T@f$ is the number of vectors.
 * The output is the @f$M@f$-th order codebook vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{c}_0, & \boldsymbol{c}_1, & \ldots, & \boldsymbol{c}_{I-1},
 *   \end{array}
 * @f]
 * where @f$I@f$ is the codebook size. The codebook size is determined by the
 * given initial codebook size @f$I_0@f$ and target codebook size @f$I_E@f$.
 * In the implemented algorithm, codebook size is repeatedly doubled from the
 * initial codebook size while @f$I < I_E@f$.
 *
 * The codebook is generated by the following algorithm:
 * - Step 0: Set @f$I \leftarrow I_0@f$.
 * - Step 1: Split the codebook vectors as
 * @f[
 *   \boldsymbol{c}_i = \left\{ \begin{array}{ll}
 *     \boldsymbol{c}_i + r \boldsymbol{\epsilon}, & 0 \le i < I \\
 *     \boldsymbol{c}_{i-I} - r \boldsymbol{\epsilon}, & I \le i < 2I
 *   \end{array} \right.
 * @f]
 * where @f$\boldsymbol{\epsilon}@f$ is a @f$M@f$-th order vector of random
 * numbers and @f$r@f$ is the splitting factor.
 * - Step 2: Update the codebook @f$N@f$ times until the convergence is reached.
 * The stop criterion is
 * @f[
 *   \left| \frac{D_{n-1} - D_{n}}{D_{n}} \right| < \varepsilon
 * @f]
 * where @f$D_{n}@f$ is the total distance between the updated codebook vectors
 * at @f$n@f$-th iteration and the input vectors.
 * - Step 3: If the number of vectors in a cluster @f$j@f$ is less than the
 * pre-determined threshold value @f$V@f$, the corresponding codebook vector is
 * updated as
 * @f{eqnarray}{
 *   \boldsymbol{c}_j &=&
 *     \boldsymbol{c}_{i_{max}} - r \boldsymbol{\epsilon}, \\
 *   \boldsymbol{c}_{i_{max}} &=&
 *     \boldsymbol{c}_{i_{max}} + r \boldsymbol{\epsilon},
 * @f}
 * where @f$i_{max}@f$ is the index of the cluster that have the largest number
 * of input vectors.
 * - Step 4: Set @f$I \leftarrow 2I@f$. If @f$I \ge I_E@f$ exit, otherwise go to
 * Step 1.
 */
class LindeBuzoGrayAlgorithm {
 public:
  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] initial_codebook_size Initial codebook size, @f$I_0@f$.
   * @param[in] target_codebook_size Target codebook size, @f$I_E@f$.
   * @param[in] min_num_vector_in_cluster Lower bound of number of vectors in a
   *            cluster, @f$V@f$.
   * @param[in] num_iteration Number of iterations, @f$N@f$.
   * @param[in] convergence_threshold Convergence threshold, @f$\varepsilon@f$.
   * @param[in] splitting_factor Splitting factor, @f$r@f$.
   * @param[in] seed Random seed.
   */
  LindeBuzoGrayAlgorithm(int num_order, int initial_codebook_size,
                         int target_codebook_size,
                         int min_num_vector_in_cluster, int num_iteration,
                         double convergence_threshold, double splitting_factor,
                         int seed);

  virtual ~LindeBuzoGrayAlgorithm() {
  }

  /**
   * @return Order of vector.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Initial codebook size.
   */
  int GetInitialCodeBookSize() const {
    return initial_codebook_size_;
  }

  /**
   * @return Target codebook size.
   */
  int GetTargetCodeBookSize() const {
    return target_codebook_size_;
  }

  /**
   * @return Minimum number of vectors in a cluster.
   */
  int GetMinNumVectorInCluster() const {
    return min_num_vector_in_cluster_;
  }

  /**
   * @return Number of iterations.
   */
  int GetNumIteration() const {
    return num_iteration_;
  }

  /**
   * @return Convergence threshold.
   */
  double GetConvergenceThreshold() const {
    return convergence_threshold_;
  }

  /**
   * @return Splitting factor.
   */
  double GetSplittingFactor() const {
    return splitting_factor_;
  }

  /**
   * @return Random seed.
   */
  int GetSeed() const {
    return seed_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input_vectors @f$M@f$-th order input vectors.
   *            The shape is @f$[T, M+1]@f$.
   * @param[in,out] codebook_vectors @f$M@f$-th order codebook vectors.
   *                The shape is @f$[I, M+1]@f$.
   * @param[out] codebook_indices @f$T@f$ codebook indices.
   * @param[out] total_distance Total distance.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<std::vector<double> >& input_vectors,
           std::vector<std::vector<double> >* codebook_vectors,
           std::vector<int>* codebook_indices, double* total_distance) const;

 private:
  const int num_order_;
  const int initial_codebook_size_;
  const int target_codebook_size_;
  const int min_num_vector_in_cluster_;
  const int num_iteration_;
  const double convergence_threshold_;
  const double splitting_factor_;
  const int seed_;

  const DistanceCalculation distance_calculation_;
  const StatisticsAccumulation statistics_accumulation_;
  const VectorQuantization vector_quantization_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(LindeBuzoGrayAlgorithm);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_LINDE_BUZO_GRAY_ALGORITHM_H_
