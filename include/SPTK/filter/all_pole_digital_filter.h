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

#ifndef SPTK_FILTER_ALL_POLE_DIGITAL_FILTER_H_
#define SPTK_FILTER_ALL_POLE_DIGITAL_FILTER_H_

#include <algorithm>  // std::fill
#include <vector>     // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class AllPoleDigitalFilter {
 public:
  class StoredSignals {
   public:
    //
    StoredSignals() {
    }

    //
    ~StoredSignals() {
    }

   private:
    //
    std::vector<double> signals_;

    //
    friend class AllPoleDigitalFilter;

    //
    DISALLOW_COPY_AND_ASSIGN(StoredSignals);
  };

  //
  AllPoleDigitalFilter(int num_filter_order, bool transposition)
      : num_filter_order_(num_filter_order),
        transposition_(transposition),
        is_valid_(true) {
    if (num_filter_order_ < 0) {
      is_valid_ = false;
    }
  }

  //
  virtual ~AllPoleDigitalFilter() {
  }

  //
  int GetNumFilterOrder() const {
    return num_filter_order_;
  }

  //
  bool GetTranspositionFlag() const {
    return transposition_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(const std::vector<double>& filter_coefficients, double filter_input,
           double* filter_output,
           AllPoleDigitalFilter::StoredSignals* signals) const;

 private:
  //
  const int num_filter_order_;

  //
  const bool transposition_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(AllPoleDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_ALL_POLE_DIGITAL_FILTER_H_
