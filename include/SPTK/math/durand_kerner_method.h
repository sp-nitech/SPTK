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
//                1996-2017  Nagoya Institute of Technology          //
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

#ifndef SPTK_MATH_DURAND_KERNER_METHOD_H_
#define SPTK_MATH_DURAND_KERNER_METHOD_H_

#include <complex>  // std::complex
#include <vector>   // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class DurandKernerMethod {
 public:
  //
  DurandKernerMethod(int num_order, int num_iteration,
                     double convergence_threshold);

  //
  virtual ~DurandKernerMethod() {
  }

  //
  int GetNumOrder() const {
    return num_order_;
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
  bool IsValid() const {
    return is_valid_;
  }

  // Assume polynomial x^n + a_1 * x^{n-1} + ... + a_n,
  // where the coefficients are [a_1 a_2 a_3 ... a_n].
  bool Run(const std::vector<double>& coefficients,
           std::vector<std::complex<double> >* roots,
           bool* is_converged) const;

 private:
  //
  const int num_order_;

  //
  const int num_iteration_;

  //
  const double convergence_threshold_;

  //
  bool is_valid_;

  //
  std::vector<double> cosine_table_;

  //
  std::vector<double> sine_table_;

  //
  DISALLOW_COPY_AND_ASSIGN(DurandKernerMethod);
};

}  // namespace sptk

#endif  // SPTK_MATH_DURAND_KERNER_METHOD_H_
