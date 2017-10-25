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

#ifndef SPTK_UTILS_UINT24_T_H_
#define SPTK_UTILS_UINT24_T_H_

#include <cstdint>  // uint8_t

namespace sptk {

static const int UINT24_MAX(16777215);

class uint24_t {
 public:
  uint24_t() {
  }

  explicit uint24_t(int initial_value) {
    *this = initial_value;
  }

  ~uint24_t() {
  }

  operator int() const {
    return (value[2] << 16) | (value[1] << 8) | (value[0] << 0);
  }

  explicit operator double() const {
    return static_cast<double>(this->operator int());
  }

  uint24_t& operator=(const uint24_t& input) {
    value[0] = input.value[0];
    value[1] = input.value[1];
    value[2] = input.value[2];
    return *this;
  }

  uint24_t& operator=(int input) {
    value[0] = (reinterpret_cast<uint8_t*>(&input))[0];
    value[1] = (reinterpret_cast<uint8_t*>(&input))[1];
    value[2] = (reinterpret_cast<uint8_t*>(&input))[2];
    return *this;
  }

  uint24_t operator-() const {
    return uint24_t(-static_cast<int>(*this));
  }

  uint24_t operator+(const uint24_t& input) const {
    return uint24_t(static_cast<int>(*this) + static_cast<int>(input));
  }

  uint24_t operator-(const uint24_t& input) const {
    return uint24_t(static_cast<int>(*this) - static_cast<int>(input));
  }

  uint24_t operator*(const uint24_t& input) const {
    return uint24_t(static_cast<int>(*this) * static_cast<int>(input));
  }

  uint24_t operator/(const uint24_t& input) const {
    return uint24_t(static_cast<int>(*this) / static_cast<int>(input));
  }

  uint24_t operator+(int input) const {
    return uint24_t(static_cast<int>(*this) + input);
  }

  uint24_t operator-(int input) const {
    return uint24_t(static_cast<int>(*this) - input);
  }

  uint24_t operator*(int input) const {
    return uint24_t(static_cast<int>(*this) * input);
  }

  uint24_t operator/(int input) const {
    return uint24_t(static_cast<int>(*this) / input);
  }

  uint24_t operator+(double input) const {
    return uint24_t(static_cast<double>(*this) + input);
  }

  uint24_t operator-(double input) const {
    return uint24_t(static_cast<double>(*this) - input);
  }

  uint24_t operator*(double input) const {
    return uint24_t(static_cast<double>(*this) * input);
  }

  uint24_t operator/(double input) const {
    return uint24_t(static_cast<double>(*this) / input);
  }

  uint24_t& operator+=(const uint24_t& input) {
    *this = *this + input;
    return *this;
  }

  uint24_t& operator-=(const uint24_t& input) {
    *this = *this - input;
    return *this;
  }

  uint24_t& operator*=(const uint24_t& input) {
    *this = *this * input;
    return *this;
  }

  uint24_t& operator/=(const uint24_t& input) {
    *this = *this / input;
    return *this;
  }

  uint24_t& operator+=(int input) {
    *this = *this + input;
    return *this;
  }

  uint24_t& operator-=(int input) {
    *this = *this - input;
    return *this;
  }

  uint24_t& operator*=(int input) {
    *this = *this * input;
    return *this;
  }

  uint24_t& operator/=(int input) {
    *this = *this / input;
    return *this;
  }

  uint24_t& operator+=(double input) {
    *this = *this + input;
    return *this;
  }

  uint24_t& operator-=(double input) {
    *this = *this - input;
    return *this;
  }

  uint24_t& operator*=(double input) {
    *this = *this * input;
    return *this;
  }

  uint24_t& operator/=(double input) {
    *this = *this / input;
    return *this;
  }

 protected:
  uint8_t value[3];
};

}  // namespace sptk

#endif  // SPTK_UTILS_UINT24_T_H_
