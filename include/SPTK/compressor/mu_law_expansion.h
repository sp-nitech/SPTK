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

#ifndef SPTK_COMPRESSOR_MU_LAW_EXPANSION_H_
#define SPTK_COMPRESSOR_MU_LAW_EXPANSION_H_

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Nonlinearly decompress data based on \f$\mu\f$-law algorithm.
 *
 * Given the input data \f$y(n)\f$, the expansion is performed as follows:
 * \f[
 *   x(n) = V \, \mathrm{sgn}(y(n))
 *     \frac{(1 + \mu)^{|y(n)|/V} - 1}{\mu}
 * \f]
 * where \f$V\f$ is the absolute maximum value of the input data and \f$\mu\f$
 * is the compression factor, which is typically set to 255.
 */
class MuLawExpansion {
 public:
  /**
   * @param[in] abs_max_value Absolute maximum value.
   * @param[in] compression_factor Compression factor, \f$\mu\f$.
   */
  MuLawExpansion(double abs_max_value, double compression_factor);

  virtual ~MuLawExpansion() {
  }

  /**
   * @return Absolute maximum value.
   */
  double GetAbsMaxValue() const {
    return abs_max_value_;
  }

  /**
   * @return Compression factor.
   */
  double GetCompressionFactor() const {
    return compression_factor_;
  }

  /**
   * @return True if this obejct is valid.
   */
  bool IsValid() const {
    return is_valid_;
  }

  /**
   * @param[in] input Input data.
   * @param[out] output Output data.
   * @return True on success, false on failure.
   */
  bool Run(double input, double* output) const;

  /**
   * @param[in] input_and_output Input/output data.
   * @return True on success, false on failure.
   */
  bool Run(double* input_and_output) const;

 private:
  const double abs_max_value_;
  const double compression_factor_;
  const double constant_;

  bool is_valid_;

  DISALLOW_COPY_AND_ASSIGN(MuLawExpansion);
};

}  // namespace sptk

#endif  // SPTK_COMPRESSOR_MU_LAW_EXPANSION_H_
