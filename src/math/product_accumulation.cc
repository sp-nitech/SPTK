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

#include "SPTK/math/product_accumulation.h"

#include <algorithm>   // std::copy, std::transform
#include <cmath>       // std::exp, std::log, std::pow
#include <cstddef>     // std::size_t
#include <functional>  // std::multiplies
#include <vector>      // std::vector

namespace sptk {

ProductAccumulation::ProductAccumulation(int num_order, bool numerically_stable)
    : num_order_(num_order),
      numerically_stable_(numerically_stable),
      is_valid_(true) {
  if (num_order_ < 0) {
    is_valid_ = false;
    return;
  }
}

bool ProductAccumulation::GetNumData(const ProductAccumulation::Buffer& buffer,
                                     int* num_data) const {
  if (!is_valid_ || NULL == num_data) {
    return false;
  }

  *num_data = buffer.num_data_;

  return true;
}

bool ProductAccumulation::GetProduct(const ProductAccumulation::Buffer& buffer,
                                     std::vector<double>* product) const {
  if (!is_valid_ || NULL == product) {
    return false;
  }

  if (product->size() != static_cast<std::size_t>(num_order_ + 1)) {
    product->resize(num_order_ + 1);
  }

  if (numerically_stable_) {
    std::transform(buffer.interim_.begin(), buffer.interim_.end(),
                   product->begin(),
                   [](double x) { return std::exp(x - 1.0); });
  } else {
    std::copy(buffer.interim_.begin(), buffer.interim_.end(), product->begin());
  }

  return true;
}

bool ProductAccumulation::GetGeometricMean(
    const ProductAccumulation::Buffer& buffer,
    std::vector<double>* mean) const {
  if (!is_valid_ || NULL == mean) {
    return false;
  }

  if (mean->size() != static_cast<std::size_t>(num_order_ + 1)) {
    mean->resize(num_order_ + 1);
  }

  const double z(1.0 / buffer.num_data_);
  if (numerically_stable_) {
    std::transform(buffer.interim_.begin(), buffer.interim_.end(),
                   mean->begin(),
                   [z](double x) { return std::exp((x - 1.0) * z); });
  } else {
    std::transform(buffer.interim_.begin(), buffer.interim_.end(),
                   mean->begin(), [z](double x) { return std::pow(x, z); });
  }

  return true;
}

void ProductAccumulation::Clear(ProductAccumulation::Buffer* buffer) const {
  if (!is_valid_ || NULL != buffer) buffer->Clear();
}

bool ProductAccumulation::Run(const std::vector<double>& data,
                              ProductAccumulation::Buffer* buffer) const {
  // Check inputs.
  const std::size_t length(num_order_ + 1);
  if (!is_valid_ || data.size() != length || NULL == buffer) {
    return false;
  }

  // Prepare memories.
  if (buffer->interim_.size() != length) {
    buffer->interim_.resize(length);
    std::fill(buffer->interim_.begin(), buffer->interim_.end(), 1.0);
  }

  ++(buffer->num_data_);

  if (numerically_stable_) {
    std::transform(data.begin(), data.end(), buffer->interim_.begin(),
                   buffer->interim_.begin(),
                   [](double x, double y) { return std::log(x) + y; });
  } else {
    std::transform(data.begin(), data.end(), buffer->interim_.begin(),
                   buffer->interim_.begin(), std::multiplies<double>());
  }

  return true;
}

}  // namespace sptk
