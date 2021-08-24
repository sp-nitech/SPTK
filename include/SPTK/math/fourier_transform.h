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

#ifndef SPTK_MATH_FOURIER_TRANSFORM_H_
#define SPTK_MATH_FOURIER_TRANSFORM_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Fourier transform wrapper.
 */
class FourierTransform {
 public:
  /**
   * Inteface of Fourier transform.
   */
  class FourierTransformInterface {
   public:
    virtual ~FourierTransformInterface() {
    }

    /**
     * @return DFT length.
     */
    virtual int GetLength() const = 0;

    /**
     * @return True if this object is valid.
     */
    virtual bool IsValid() const = 0;

    /**
     * @param[in] real_part_input @f$L@f$-length real part of input.
     * @param[in] imag_part_input @f$L@f$-length imaginary part of input.
     * @param[out] real_part_output @f$L@f$-length real part of output.
     * @param[out] imag_part_output @f$L@f$-length imaginary part of output.
     * @return True on success, false on failure.
     */
    virtual bool Run(const std::vector<double>& real_part_input,
                     const std::vector<double>& imag_part_input,
                     std::vector<double>* real_part_output,
                     std::vector<double>* imag_part_output) const = 0;

    /**
     * @param[in,out] real_part Real part.
     * @param[in,out] imag_part Imaginary part.
     * @return True on success, false on failure.
     */
    virtual bool Run(std::vector<double>* real_part,
                     std::vector<double>* imag_part) const = 0;
  };

  /**
   * @param[in] length DFT length, @f$L@f$.
   */
  explicit FourierTransform(int length);

  ~FourierTransform() {
    delete fourier_transform_;
  }

  /**
   * @return DFT length.
   */
  int GetLength() const {
    return fourier_transform_->GetLength();
  }

  /**
   * @return True if this object is valid.
   */
  bool IsValid() const {
    return fourier_transform_->IsValid();
  }

  /**
   * @param[in] real_part_input @f$L@f$-length real part of input.
   * @param[in] imag_part_input @f$L@f$-length imaginary part of input.
   * @param[out] real_part_output @f$L@f$-length real part of output.
   * @param[out] imag_part_output @f$L@f$-length imaginary part of output.
   * @return True on success, false on failure.
   */
  bool Run(const std::vector<double>& real_part_input,
           const std::vector<double>& imag_part_input,
           std::vector<double>* real_part_output,
           std::vector<double>* imag_part_output) const {
    return fourier_transform_->Run(real_part_input, imag_part_input,
                                   real_part_output, imag_part_output);
  }

  /**
   * @param[in,out] real_part Real part.
   * @param[in,out] imag_part Imaginary part.
   * @return True on success, false on failure.
   */
  bool Run(std::vector<double>* real_part,
           std::vector<double>* imag_part) const {
    return fourier_transform_->Run(real_part, imag_part);
  }

 private:
  FourierTransformInterface* fourier_transform_;

  DISALLOW_COPY_AND_ASSIGN(FourierTransform);
};

}  // namespace sptk

#endif  // SPTK_MATH_FOURIER_TRANSFORM_H_
