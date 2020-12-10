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

#ifndef SPTK_COMPRESSION_MULTISTAGE_VECTOR_QUANTIZATION_H_
#define SPTK_COMPRESSION_MULTISTAGE_VECTOR_QUANTIZATION_H_

#include <vector>  // std::vector

#include "SPTK/compression/vector_quantization.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform multistage vector quantization.
 *
 * The input is the @f$M@f$-th order vector:
 * @f[
 *   \begin{array}{cccc}
 *     x(0), & x(1), & \ldots, & x(M),
 *   \end{array}
 * @f]
 * and the @f$M@f$-th order @f$N \times I@f$ codebook vectors,
 * @f$\left\{ c_i^{(n)}(m) \right\}@f$.
 * The output is the @f$N@f$ codebook indices:
 * @f[
 *   \begin{array}{cccc}
 *     i(1), & i(2), & \ldots, & i(N),
 *   \end{array}
 * @f]
 * where
 * @f[
 *   i(n) = \mathop{\mathrm{argmin}}_j \sum_{j=0}^{I-1} \sum_{m=0}^M
 *       (e^{(n)}(m) - c_j^{(n)}(m))^2,
 * @f]
 * and the quantization error is
 * @f[
 *   e^{(n)}(m) = \left\{ \begin{array}{ll}
 *     x(m), & n = 1 \\
 *     e^{(n-1)}(m) - c_j^{(n-1)}(m). & n > 1 \\
 *   \end{array} \right.
 * @f]
 */
class MultistageVectorQuantization {
 public:
  /**
   * Buffer for MultistageVectorQuantization class.
   */
  class Buffer {
   public:
    Buffer() {
    }

    virtual ~Buffer() {
    }

   private:
    std::vector<double> quantization_error_;

    friend class MultistageVectorQuantization;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  /**
   * @param[in] num_order Order of vector, @f$M@f$.
   * @param[in] num_stage Number of quantization stages, @f$N@f$.
   */
  MultistageVectorQuantization(int num_order, int num_stage);

  virtual ~MultistageVectorQuantization() {
  }

  /**
   * @return Order of vector.
   */
  int GetNumOrder() const {
    return num_order_;
  }

  /**
   * @return Number of stages.
   */
  int GetNumStage() const {
    return num_stage_;
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input_vector @f$M@f$-th order input vector.
   * @param[in] codebook_vectors @f$M@f$-th order @f$I@f$ codebook vectors.
   *             The shape is @f$[N, I, M+1]@f$.
   * @param[out] codebook_indices @f$N@f$ codebook indices.
   * @param[out] buffer Buffer.
   * @return True on success, false on failure.
   */
  bool Run(
      const std::vector<double>& input_vector,
      const std::vector<std::vector<std::vector<double> > >& codebook_vectors,
      std::vector<int>* codebook_indices,
      MultistageVectorQuantization::Buffer* buffer) const;

 private:
  const int num_order_;
  const int num_stage_;
  const VectorQuantization vector_quantization_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MultistageVectorQuantization);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSION_MULTISTAGE_VECTOR_QUANTIZATION_H_
