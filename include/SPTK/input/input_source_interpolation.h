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

#ifndef SPTK_INPUT_INPUT_SOURCE_INTERPOLATION_H_
#define SPTK_INPUT_INPUT_SOURCE_INTERPOLATION_H_

#include <vector>  // std::vector

#include "SPTK/input/input_source_interface.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class InputSourceInterpolation : public InputSourceInterface {
 public:
  //
  InputSourceInterpolation(int frame_period, int interpolation_period,
                           bool use_final_frame_for_exceeded_frame,
                           InputSourceInterface* source);

  //
  virtual ~InputSourceInterpolation() {
  }

  //
  int GetFramePeriod() const {
    return frame_period_;
  }

  //
  int GetInterpolationPeriod() const {
    return interpolation_period_;
  }

  //
  bool UseFinalFrameForExceededFrame() const {
    return use_final_frame_for_exceeded_frame_;
  }

  //
  virtual int GetSize() const {
    return source_ ? source_->GetSize() : 0;
  }

  //
  virtual bool IsValid() const {
    return is_valid_;
  }

  //
  virtual bool Get(std::vector<double>* buffer);

 private:
  //
  void CalculateIncrement();

  //
  const int frame_period_;

  //
  const int interpolation_period_;

  //
  const int first_interpolation_period_;

  //
  const bool use_final_frame_for_exceeded_frame_;

  //
  int remained_num_samples_;

  //
  int data_length_;

  //
  int point_index_in_frame_;

  //
  InputSourceInterface* source_;

  //
  bool is_valid_;

  //
  std::vector<double> curr_data_;

  //
  std::vector<double> next_data_;

  //
  std::vector<double> increment_;

  //
  DISALLOW_COPY_AND_ASSIGN(InputSourceInterpolation);
};

}  // namespace sptk

#endif  // SPTK_INPUT_INPUT_SOURCE_INTERPOLATION_H_
