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

#ifndef SPTK_MATH_ENTROPY_CALCULATION_H_
#define SPTK_MATH_ENTROPY_CALCULATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Calculate entropy.
 *
 * The input is the probabilities of \f$N\f$ events:
 * \f[
 *   \begin{array}{cccc}
 *     p(1), & p(2), & \ldots, & p(N),
 *   \end{array}
 * \f]
 * where
 * \f[
 *   \sum_{n=1}^N \, p(n) = 1.
 * \f]
 * The output is the entropy for the probabilities:
 * \f[
 *   \begin{array}{cccc}
 *     H = -\displaystyle\sum_{n=1}^N p(n) \log_b p(n)
 *   \end{array}
 * \f]
 * where \f$b\f$ is 2, \f$e\f$, or 10.
 */
class EntropyCalculation {
 public:
  /**
   * Unit of entropy.
   */
  enum EntropyUnits { kBit = 0, kNat, kDit, kNumUnits };

  /**
   * @param[in] num_element Number of elements, \f$N\f$.
   * @param[in] entropy_unit Unit of entropy.
   */
  EntropyCalculation(int num_element, EntropyUnits entropy_unit);

  virtual ~EntropyCalculation() {
  }

  /**
   * @return Number of elements.
   */
  int GetNumElement() const {
    return num_element_;
  }

  /**
   * @return Unit of entropy.
   */
  EntropyUnits GetEntropyUnit() const {
    return entropy_unit_;
  }

  /**
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] probability Probabiltiy distribution.
   * @param[out] entropy Entropy.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& probability, double* entropy) const;

 private:
  const int num_element_;
  const EntropyUnits entropy_unit_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(EntropyCalculation);
};

}  // namespace sptk

#endif  // SPTK_MATH_ENTROPY_CALCULATION_H_
