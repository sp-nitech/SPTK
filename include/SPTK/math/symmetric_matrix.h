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
//                1996-2018  Nagoya Institute of Technology          //
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

#ifndef SPTK_MATH_SYMMETRIC_MATRIX_H_
#define SPTK_MATH_SYMMETRIC_MATRIX_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class SymmetricMatrix {
 public:
  class Row {
   public:
    Row(const SymmetricMatrix& symmetric_matrix, int row)
        : symmetric_matrix_(symmetric_matrix), row_(row) {
    }
    virtual ~Row() {
    }
    double& operator[](int column);
    const double& operator[](int column) const;

   private:
    const SymmetricMatrix& symmetric_matrix_;
    const int row_;
    friend class SymmetricMatrix;
    DISALLOW_COPY_AND_ASSIGN(Row);
  };

  //
  explicit SymmetricMatrix(int num_dimension = 0);

  //
  SymmetricMatrix(const SymmetricMatrix& symmetric_matrix);

  //
  SymmetricMatrix& operator=(const SymmetricMatrix& symmetric_matrix);

  //
  virtual ~SymmetricMatrix() {
  }

  //
  int GetNumDimension() const {
    return num_dimension_;
  }

  //
  void Resize(int num_dimension);

  //
  Row operator[](int row) {
    return Row(*this, row);
  }

  //
  const Row operator[](int row) const {
    return Row(*this, row);
  }

  //
  double& At(int row, int column);

  //
  const double& At(int row, int column) const;

  //
  void FillZero();

  //
  bool Invert(SymmetricMatrix* inverse_matrix) const;

 private:
  //
  int num_dimension_;

  //
  std::vector<double> data_;

  //
  std::vector<double*> index_;
};

}  // namespace sptk

#endif  // SPTK_MATH_SYMMETRIC_MATRIX_H_
