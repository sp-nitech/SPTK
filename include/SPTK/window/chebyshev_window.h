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

#ifndef SPTK_WINDOW_CHEBYSHEV_WINDOW_H_
#define SPTK_WINDOW_CHEBYSHEV_WINDOW_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"
#include "SPTK/window/window_interface.h"

namespace sptk {

/**
 * Generate Chebyshev window.
 */
class ChebyshevWindow : public WindowInterface {
 public:
  /**
   * @param[in] window_length Window length.
   * @param[in] ripple_ratio Ripple ratio.
   * @param[in] periodic Whether to generate a periodic window.
   */
  ChebyshevWindow(int window_length, double ripple_ratio, bool periodic);

  virtual ~ChebyshevWindow() {
  }

  /**
   * @param[in] attenuation Stopband attenuation in dB.
   * @return Ripple ratio.
   */
  static double AttenuationToRippleRatio(double attenuation);

  /**
   * @return Window length.
   */
  virtual int GetWindowLength() const {
    return window_length_;
  }

  /**
   * @return Ripple ratio.
   */
  double GetRippleRatio() const {
    return ripple_ratio_;
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
   * @return Chebyshev window.
   */
  virtual const std::vector<double>& Get() const {
    return window_;
  }

 private:
  const int window_length_;
  const double ripple_ratio_;
  const bool periodic_;

  bool is_valid_;

  std::vector<double> window_;

  DISALLOW_COPY_AND_ASSIGN(ChebyshevWindow);
};

}  // namespace sptk

#endif  // SPTK_WINDOW_CHEBYSHEV_WINDOW_H_
