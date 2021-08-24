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
