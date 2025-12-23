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

#ifndef SPTK_RESAMPLER_RESAMPLER_INTERFACE_H_
#define SPTK_RESAMPLER_RESAMPLER_INTERFACE_H_

#include <vector>  // std::vector

namespace sptk {

/**
 * An interface of resampler.
 */
class ResamplerInterface {
 public:
  virtual ~ResamplerInterface() = default;

  /**
   * @return Latency introduced by resampling.
   */
  virtual int GetLatency() const = 0;

  /**
   * Clears internal state.
   */
  virtual void Clear() = 0;

  /**
   * @return True if this object is valid.
   */
  virtual bool IsValid() const = 0;

  /**
   * @param[in] inputs Input samples.
   * @param[out] outputs Output samples.
   * @return True on success, false on failure.
   */
  virtual bool Get(const std::vector<double>& inputs,
                   std::vector<double>* outputs) = 0;
};

}  // namespace sptk

#endif  // SPTK_RESAMPLER_RESAMPLER_INTERFACE_H_
