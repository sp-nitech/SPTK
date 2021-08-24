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

#ifndef SPTK_UTILS_INT24_T_H_
#define SPTK_UTILS_INT24_T_H_

#include <cstdint>  // uint8_t

namespace sptk {

static const int INT24_MAX(8388607);
static const int INT24_MIN(-8388608);

class int24_t {
 public:
  int24_t() {
  }

  explicit int24_t(int initial_value) {
    *this = initial_value;
  }

  ~int24_t() {
  }

  operator int() const {
    if (value[2] & 0x80) {
      return (0xff << 24) | (value[2] << 16) | (value[1] << 8) |
             (value[0] << 0);
    }
    return (value[2] << 16) | (value[1] << 8) | (value[0] << 0);
  }

  explicit operator double() const {
    return static_cast<double>(this->operator int());
  }

  int24_t& operator=(const int24_t& input) {
    value[0] = input.value[0];
    value[1] = input.value[1];
    value[2] = input.value[2];
    return *this;
  }

  int24_t& operator=(int input) {
    value[0] = (reinterpret_cast<uint8_t*>(&input))[0];
    value[1] = (reinterpret_cast<uint8_t*>(&input))[1];
    value[2] = (reinterpret_cast<uint8_t*>(&input))[2];
    return *this;
  }

  int24_t operator-() const {
    return int24_t(-static_cast<int>(*this));
  }

  int24_t operator+(const int24_t& input) const {
    return int24_t(static_cast<int>(*this) + static_cast<int>(input));
  }

  int24_t operator-(const int24_t& input) const {
    return int24_t(static_cast<int>(*this) - static_cast<int>(input));
  }

  int24_t operator*(const int24_t& input) const {
    return int24_t(static_cast<int>(*this) * static_cast<int>(input));
  }

  int24_t operator/(const int24_t& input) const {
    return int24_t(static_cast<int>(*this) / static_cast<int>(input));
  }

  int24_t operator+(int input) const {
    return int24_t(static_cast<int>(*this) + input);
  }

  int24_t operator-(int input) const {
    return int24_t(static_cast<int>(*this) - input);
  }

  int24_t operator*(int input) const {
    return int24_t(static_cast<int>(*this) * input);
  }

  int24_t operator/(int input) const {
    return int24_t(static_cast<int>(*this) / input);
  }

  int24_t operator+(double input) const {
    return int24_t(static_cast<double>(*this) + input);
  }

  int24_t operator-(double input) const {
    return int24_t(static_cast<double>(*this) - input);
  }

  int24_t operator*(double input) const {
    return int24_t(static_cast<double>(*this) * input);
  }

  int24_t operator/(double input) const {
    return int24_t(static_cast<double>(*this) / input);
  }

  int24_t& operator+=(const int24_t& input) {
    *this = *this + input;
    return *this;
  }

  int24_t& operator-=(const int24_t& input) {
    *this = *this - input;
    return *this;
  }

  int24_t& operator*=(const int24_t& input) {
    *this = *this * input;
    return *this;
  }

  int24_t& operator/=(const int24_t& input) {
    *this = *this / input;
    return *this;
  }

  int24_t& operator+=(int input) {
    *this = *this + input;
    return *this;
  }

  int24_t& operator-=(int input) {
    *this = *this - input;
    return *this;
  }

  int24_t& operator*=(int input) {
    *this = *this * input;
    return *this;
  }

  int24_t& operator/=(int input) {
    *this = *this / input;
    return *this;
  }

  int24_t& operator+=(double input) {
    *this = *this + input;
    return *this;
  }

  int24_t& operator-=(double input) {
    *this = *this - input;
    return *this;
  }

  int24_t& operator*=(double input) {
    *this = *this * input;
    return *this;
  }

  int24_t& operator/=(double input) {
    *this = *this / input;
    return *this;
  }

 protected:
  uint8_t value[3];
};

}  // namespace sptk

#endif  // SPTK_UTILS_INT24_T_H_
