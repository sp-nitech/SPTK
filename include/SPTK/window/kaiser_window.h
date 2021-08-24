// ------------------------------------------------------------------------ //
// Copyright 2021 SPTK Working Group                                        //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ------------------------------------------------------------------------ //

#ifndef SPTK_WINDOW_KAISER_WINDOW_H_
#define SPTK_WINDOW_KAISER_WINDOW_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"
#include "SPTK/window/window_interface.h"

namespace sptk {

/**
 * Generate Kaiser window.
 */
class KaiserWindow : public WindowInterface {
 public:
  /**
   * @param[in] window_length Window length.
   * @param[in] beta Window coefficient.
   * @param[in] periodic Whether to generate a periodic window.
   */
  KaiserWindow(int window_length, double beta, bool periodic);

  virtual ~KaiserWindow() {
  }

  /**
   * @param[in] attenuation Stopband attenuation in dB.
   * @return Beta.
   */
  static double AttenuationToBeta(double attenuation);

  /**
   * @return Window length.
   */
  virtual int GetWindowLength() const {
    return window_length_;
  }

  /**
   * @return Beta.
   */
  double GetBeta() const {
    return beta_;
  }

  /**
   * @return True if window is periodic.
   */
  bool IsPeriodic() const {
    return periodic_;
  }

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const {
    return is_valid_;
  }

  /**
   * @return Kaiser window.
   */
  virtual const std::vector<double>& Get() const {
    return window_;
  }

 private:
  const int window_length_;
  const double beta_;
  const bool periodic_;

  bool is_valid_;

  std::vector<double> window_;

  DISALLOW_COPY_AND_ASSIGN(KaiserWindow);
};

}  // namespace sptk

#endif  // SPTK_WINDOW_KAISER_WINDOW_H_
