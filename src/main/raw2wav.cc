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

#include <cstddef>      // std::size_t
#include <cstdint>      // std::int16_t, std::uint8_t
#include <cstring>      // std::strncmp
#include <fstream>      // std::ifstream
#include <iomanip>      // std::setw
#include <iostream>     // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>      // std::ostringstream
#include <string>       // std::string
#include <type_traits>  // std::is_same
#include <vector>       // std::vector

#include "GETOPT/ya_getopt.h"
#include "SPTK/utils/sptk_utils.h"

#ifndef CPPCHECK
#define DR_WAV_IMPLEMENTATION
#define DR_WAV_NO_WCHAR
#include "dr_libs/dr_wav.h"
#endif  // CPPCHECK

namespace {

enum OutputFormats { kWav = 0, kNumOutputFormats };

const double kDefaultSamplingRate(16.0);
const int kDefaultNumChannels(1);
const OutputFormats kDefaultOutputFormat(kWav);
const char* kDefaultInputDataType("s");

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " raw2wav - convert raw data format to audio file" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       raw2wav [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -s s  : sampling rate [kHz] (double)[" << std::setw(5) << std::right << kDefaultSamplingRate  << "][ 0 <  s <=   ]" << std::endl;  // NOLINT
  *stream << "       -c c  : number of channels  (   int)[" << std::setw(5) << std::right << kDefaultNumChannels   << "][ 1 <= c <=   ]" << std::endl;  // NOLINT
  *stream << "       -o o  : output format       (   int)[" << std::setw(5) << std::right << kDefaultOutputFormat  << "][ 0 <= o <= 0 ]" << std::endl;  // NOLINT
  *stream << "                 0 (WAV)" << std::endl;
  *stream << "       +type : input data type             [" << std::setw(5) << std::right << kDefaultInputDataType << "]" << std::endl;  // NOLINT
  *stream << "                 s (short, -32768 ~ 32767) " << std::endl;
  *stream << "                 f (float, -1.0 ~ 1.0) " << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       waveform                    (  type)[stdin]" <<std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       audio file" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       use sox or ffmpeg to handle other formats" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class AudioWriterInterface {
 public:
  virtual ~AudioWriterInterface() = default;

  virtual bool Initialize() = 0;

  virtual void Finalize() = 0;

  virtual bool Run(const std::vector<std::uint8_t>& data) = 0;
};

template <typename T>
class WavWriter final : public AudioWriterInterface {
 public:
  WavWriter(int sampling_rate, int num_channels) : format_() {
    format_.container = drwav_container_riff;
    format_.channels = static_cast<drwav_uint32>(num_channels);
    format_.sampleRate = static_cast<drwav_uint32>(sampling_rate);
    if (std::is_same<T, int16_t>::value) {
      format_.format = DR_WAVE_FORMAT_PCM;
      format_.bitsPerSample = 16;
    } else if (std::is_same<T, float>::value) {
      format_.format = DR_WAVE_FORMAT_IEEE_FLOAT;
      format_.bitsPerSample = 32;
    }
  }

  ~WavWriter() override {
    Finalize();
  }

  bool Initialize() override {
    Finalize();
    return true;
  }

  void Finalize() override {
  }

  bool Run(const std::vector<std::uint8_t>& data) override {
    drwav wav;
    void* buffer;
    std::size_t buffer_size;
    const std::size_t frameCount(data.size() / sizeof(T));
    if (!drwav_init_memory_write_sequential(&wav, &buffer, &buffer_size,
                                            &format_, frameCount, NULL)) {
      return false;
    }
    if (frameCount != drwav_write_pcm_frames(&wav, frameCount, data.data())) {
      return false;
    }
    drwav_uninit(&wav);
    std::cout.write(reinterpret_cast<const char*>(buffer), buffer_size);
    drwav_free(buffer, NULL);
    return true;
  }

 private:
  drwav_data_format format_;

  DISALLOW_COPY_AND_ASSIGN(WavWriter<T>);
};

class AudioWriterWrapper {
 public:
  AudioWriterWrapper(const std::string& input_data_type,
                     OutputFormats output_format, int sampling_rate,
                     int num_channels)
      : audio_writer_(NULL) {
    if ("s" == input_data_type && kWav == output_format) {
      audio_writer_ = new WavWriter<std::int16_t>(sampling_rate, num_channels);
    } else if ("f" == input_data_type && kWav == output_format) {
      audio_writer_ = new WavWriter<float>(sampling_rate, num_channels);
    }
  }

  ~AudioWriterWrapper() {
    delete audio_writer_;
  }

  bool IsValid() const {
    return NULL != audio_writer_;
  }

  bool Run(const std::vector<std::uint8_t>& data) const {
    return (IsValid() && audio_writer_->Initialize() &&
            audio_writer_->Run(data));
  }

 private:
  AudioWriterInterface* audio_writer_;

  DISALLOW_COPY_AND_ASSIGN(AudioWriterWrapper);
};

}  // namespace

/**
 * @a raw2wav [ @e option ] [ @e infile ]
 *
 * - @b -s @e double
 *   - sampling rate [kHz]
 * - @b -c @e int
 *   - number of channels
 * - @b -q @e int
 *   - output audio format
 *     \arg @c 0 WAV
 * - @b +type @e char
 *   - input data type
 *     \arg @c s short (-32768 ~ 32767)
 *     \arg @c f float (-1.0 ~ 1.0)
 * - @b infile @e str
 *   - waveform
 * - @b stdout
 *   - audio file
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  double sampling_rate(kDefaultSamplingRate);
  int num_channels(kDefaultNumChannels);
  OutputFormats output_format(kDefaultOutputFormat);
  std::string input_data_type(kDefaultInputDataType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "s:c:o:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 's': {
        if (!sptk::ConvertStringToDouble(optarg, &sampling_rate) ||
            sampling_rate <= 0.0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -s option must be a positive number";
          sptk::PrintErrorMessage("raw2wav", error_message);
          return 1;
        }
        break;
      }
      case 'c': {
        if (!sptk::ConvertStringToInteger(optarg, &num_channels) ||
            num_channels <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -c option must be a positive integer";
          sptk::PrintErrorMessage("raw2wav", error_message);
          return 1;
        }
        break;
      }
      case 'o': {
        const int min(0);
        const int max(static_cast<int>(kNumOutputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -o option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("raw2wav", error_message);
          return 1;
        }
        output_format = static_cast<OutputFormats>(tmp);
        break;
      }
      case 'h': {
        PrintUsage(&std::cout);
        return 0;
      }
      default: {
        PrintUsage(&std::cerr);
        return 1;
      }
    }
  }

  const char* input_file(NULL);
  for (int i(argc - optind); 1 <= i; --i) {
    const char* arg(argv[argc - i]);
    if (0 == std::strncmp(arg, "+", 1)) {
      const std::string str(arg);
      input_data_type = str.substr(1, std::string::npos);
    } else if (NULL == input_file) {
      input_file = arg;
    } else {
      std::ostringstream error_message;
      error_message << "Too many input files";
      sptk::PrintErrorMessage("raw2wav", error_message);
      return 1;
    }
  }

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("raw2wav", error_message);
    return 1;
  }

  std::ifstream ifs;
  if (NULL != input_file) {
    ifs.open(input_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << input_file;
      sptk::PrintErrorMessage("raw2wav", error_message);
      return 1;
    }
  }
  std::istream& input_stream(ifs.is_open() ? ifs : std::cin);

  AudioWriterWrapper audio_writer(input_data_type, output_format,
                                  static_cast<int>(1000 * sampling_rate),
                                  num_channels);
  if (!audio_writer.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected input/output format";
    sptk::PrintErrorMessage("raw2wav", error_message);
    return 1;
  }

  std::vector<std::uint8_t> waveform;
  {
    std::uint8_t tmp;
    while (sptk::ReadStream(&tmp, &input_stream)) {
      waveform.push_back(tmp);
    }
  }
  if (waveform.empty()) return 0;

  if (!audio_writer.Run(waveform)) {
    std::ostringstream error_message;
    error_message << "Failed to convert";
    sptk::PrintErrorMessage("raw2wav", error_message);
    return 1;
  }

  return 0;
}
