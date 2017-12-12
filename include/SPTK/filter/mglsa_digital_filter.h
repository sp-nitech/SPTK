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

#ifndef SPTK_FILTER_MGLSA_DIGITAL_FILTER_H_
#define SPTK_FILTER_MGLSA_DIGITAL_FILTER_H_

#include <vector>  // std::vector

#include "SPTK/filter/mlsa_digital_filter.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class MglsaDigitalFilter {
 public:
  class Buffer {
   public:
    Buffer() {
    }
    virtual ~Buffer() {
    }

   private:
    std::vector<double> signals_;
    MlsaDigitalFilter::Buffer mlsa_digital_filter_buffer_;
    friend class MglsaDigitalFilter;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  // If num_stage is 0, MLSA filter is used.
  MglsaDigitalFilter(int num_filter_order, int num_pade_order, int num_stage,
                     double alpha, bool transposition);

  //
  virtual ~MglsaDigitalFilter() {
  }

  //
  int GetNumFilterOrder() const {
    return num_filter_order_;
  }

  //
  int GetNumPadeOrder() const {
    return mlsa_digital_filter_.GetNumPadeOrder();
  }

  //
  int GetNumStage() const {
    return num_stage_;
  }

  //
  double GetAlpha() const {
    return alpha_;
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
           MglsaDigitalFilter::Buffer* stored_signals) const;

 private:
  //
  const int num_filter_order_;

  //
  const int num_stage_;

  //
  const double alpha_;

  //
  const bool transposition_;

  //
  const MlsaDigitalFilter mlsa_digital_filter_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(MglsaDigitalFilter);
};

}  // namespace sptk

#endif  // SPTK_FILTER_MGLSA_DIGITAL_FILTER_H_
