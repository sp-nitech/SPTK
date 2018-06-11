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

#ifndef SPTK_FILTER_INFINITE_IMPULSE_RESPONSE_DIGITAL_FILTER_H_
#define SPTK_FILTER_INFINITE_IMPULSE_RESPONSE_DIGITAL_FILTER_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class InfiniteImpulseResponseDigitalFilter {
 public:
  class Buffer {
   public:
    //
    Buffer() : p_(0) {
    }

    //
    virtual ~Buffer() {
    }

   private:
    //
    int p_;

    //
    std::vector<double> signals_;

    //
    friend class InfiniteImpulseResponseDigitalFilter;

    //
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  InfiniteImpulseResponseDigitalFilter(
      const std::vector<double>& denominator_filter_coefficients,
      const std::vector<double>& numerator_filter_coefficients);

  //
  virtual ~InfiniteImpulseResponseDigitalFilter() {
  }

  //
  int GetNumDenominatorFilterOrder() const {
    return num_denominator_filter_order_;
  }

  //
  int GetNumNumeratorFilterOrder() const {
    return num_numerator_filter_order_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(double filter_input, double* filter_output,
           InfiniteImpulseResponseDigitalFilter::Buffer* buffer) const;

 private:
  //
  const std::vector<double> denominator_filter_coefficients_;

  //
  const std::vector<double> numerator_filter_coefficients_;

  //
  const int num_denominator_filter_order_;

  //
  const int num_numerator_filter_order_;

  //
  const int num_filter_order_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(InfiniteImpulseResponseDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_INFINITE_IMPULSE_RESPONSE_DIGITAL_FILTER_H_
