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

#ifndef SPTK_INPUT_INPUT_SOURCE_FROM_MATRIX_H_
#define SPTK_INPUT_INPUT_SOURCE_FROM_MATRIX_H_

#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class InputSourceFromMatrix : public InputSourceInterface {
 public:
  //
  InputSourceFromMatrix(int row_size, int col_size, double** input_matrix)
      : row_size_(row_size),
        col_size_(col_size),
        input_matrix_(input_matrix),
        col_position_(0),
        is_valid_(true) {
    if (row_size_ <= 0 || col_size_ <= 0 || NULL == input_matrix_) {
      is_valid_ = false;
    }
  }

  //
  virtual ~InputSourceFromMatrix() {
  }

  //
  virtual int GetSize() const {
    return row_size_;
  }

  //
  int GetRowSize() const {
    return row_size_;
  }

  //
  int GetColSize() const {
    return col_size_;
  }

  //
  virtual bool IsValid() const {
    return is_valid_;
  }

  //
  virtual bool Get(std::vector<double>* buffer);

 private:
  //
  const int row_size_;

  //
  const int col_size_;

  //
  double** input_matrix_;

  //
  int col_position_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(InputSourceFromMatrix);
};

}  // namespace sptk

#endif  // SPTK_INPUT_INPUT_SOURCE_FROM_MATRIX_H_
