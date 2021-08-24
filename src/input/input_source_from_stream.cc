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

#include "SPTK/input/input_source_from_stream.h"

namespace sptk {

InputSourceFromStream::InputSourceFromStream(bool zero_padding, int read_size,
                                             std::istream* input_stream)
    : zero_padding_(zero_padding),
      read_size_(read_size),
      input_stream_(input_stream),
      is_valid_(true) {
  if (read_size_ <= 0 || NULL == input_stream_) {
    is_valid_ = false;
    return;
  }
}

bool InputSourceFromStream::Get(std::vector<double>* buffer) {
  if (NULL == buffer || !is_valid_) {
    return false;
  }
  return sptk::ReadStream(zero_padding_, 0, 0, read_size_, buffer,
                          input_stream_, NULL);
}

}  // namespace sptk
