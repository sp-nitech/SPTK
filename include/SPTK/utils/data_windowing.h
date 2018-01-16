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

#ifndef SPTK_UTILS_DATA_WINDOWING_H_
#define SPTK_UTILS_DATA_WINDOWING_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class DataWindowing {
 public:
  //
  enum NormalizationType {
    kNone = 0,
    kPower,
    kMagnitude,
    kNumNormalizationTypes,
  };

  //
  enum WindowType {
    kBlackman = 0,
    kHamming,
    kHanning,
    kBartlett,
    kTrapezoidal,
    kRectangular,
    kNumWindowTypes,
  };

  //
  DataWindowing(int num_input_order, int num_output_order,
                NormalizationType normalization_type, WindowType window_type);

  //
  virtual ~DataWindowing() {
  }

  //
  int GetNumInputOrder() const {
    return num_input_order_;
  }

  //
  int GetNumOutputOrder() const {
    return num_output_order_;
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(const std::vector<double>& data_sequence,
           std::vector<double>* windowed_data_sequence) const;

 private:
  //
  void CreateBlackmanWindow();
  void CreateHammingWindow();
  void CreateHanningWindow();
  void CreateBartlettWindow();
  void CreateTrapezoidalWindow();
  void CreateRectangularWindow();

  const int num_input_order_;

  //
  const int num_output_order_;

  //
  bool is_valid_;

  //
  std::vector<double> window_;

  //
  DISALLOW_COPY_AND_ASSIGN(DataWindowing);
};

}  // namespace sptk

#endif  // SPTK_UTILS_DATA_WINDOWING_H_
