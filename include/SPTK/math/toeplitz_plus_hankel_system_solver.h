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

#ifndef SPTK_MATH_TOEPLITZ_PLUS_HANKEL_SYSTEM_SOLVER_H_
#define SPTK_MATH_TOEPLITZ_PLUS_HANKEL_SYSTEM_SOLVER_H_

#include <vector>  // std::vector

#include "SPTK/math/matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class ToeplitzPlusHankelSystemSolver {
 public:
  class Buffer {
   public:
    Buffer()
        : vx_(2, 2),
          ex_(2, 2),
          ep_(2, 1),
          bx_(2, 2),
          g_(2, 1),
          inv_(2, 2),
          bar_(2, 1),
          tau_(2, 2) {
    }
    virtual ~Buffer() {
    }

   private:
    std::vector<Matrix> r_;
    std::vector<Matrix> x_;
    std::vector<Matrix> prev_x_;
    std::vector<Matrix> p_;
    Matrix b_;
    Matrix vx_;
    Matrix ex_;
    Matrix ep_;
    Matrix bx_;
    Matrix g_;
    Matrix inv_;
    Matrix bar_;
    Matrix tau_;
    friend class ToeplitzPlusHankelSystemSolver;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  explicit ToeplitzPlusHankelSystemSolver(int num_order);

  //
  virtual ~ToeplitzPlusHankelSystemSolver() {
  }

  //
  int GetNumOrder() const {
    return num_order_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(const std::vector<double>& toeplitz_coefficient_vector,
           const std::vector<double>& hankel_coefficient_vector,
           const std::vector<double>& constant_vector,
           std::vector<double>* solution_vector,
           ToeplitzPlusHankelSystemSolver::Buffer* buffer) const;

 private:
  //
  const int num_order_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(ToeplitzPlusHankelSystemSolver);
};

}  // namespace sptk

#endif  // SPTK_MATH_TOEPLITZ_PLUS_HANKEL_SYSTEM_SOLVER_H_
