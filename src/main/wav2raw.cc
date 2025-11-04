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

#include <algorithm>    // std::transform
#include <cctype>       // std::tolower
#include <cstdint>      // std::int16_t
#include <cstring>      // std::strlen, std::strncmp, std::strrchr
#include <iomanip>      // std::setw
#include <iostream>     // std::cerr, std::cin, std::cout, std::endl, etc.
#include <iterator>     // std::istreambuf_iterator
#include <sstream>      // std::ostringstream
#include <string>       // std::string
#include <type_traits>  // std::enable_if, std::is_same
#include <vector>       // std::vector

#include "GETOPT/ya_getopt.h"
#include "SPTK/utils/sptk_utils.h"

#ifndef CPPCHECK
#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_WCHAR
#include "dr_libs/dr_flac.h"
#define DR_MP3_IMPLEMENTATION
#define DR_MP3_FLOAT_OUTPUT
#include "dr_libs/dr_mp3.h"
#define DR_WAV_IMPLEMENTATION
#define DR_WAV_NO_WCHAR
#include "dr_libs/dr_wav.h"
#define STB_VORBIS_NO_PUSHDATA_API
#include "stb/stb_vorbis.h"
#endif  // CPPCHECK

namespace {

enum InputFormats { kWav = 0, kMp3, kFlac, kOgg, kNumInputFormats };

const InputFormats kDefaultInputFormat(kWav);
const char* kDefaultOutputDataType("s");

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " wav2raw - convert audio file to raw data format" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       wav2raw [ options ] [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -q q  : input format     (   int)[" << std::setw(5) << std::right << "N/A"                  << "][ 0 <= q <= 3 ]" << std::endl;  // NOLINT
  *stream << "                 0 (WAV)" << std::endl;
  *stream << "                 1 (MP3)" << std::endl;
  *stream << "                 2 (FLAC)" << std::endl;
  *stream << "                 3 (OGG)" << std::endl;
  *stream << "       +type : output data type         [" << std::setw(5) << std::right << kDefaultOutputDataType << "]" << std::endl;  // NOLINT
  *stream << "                 s (short, -32768 ~ 32767) " << std::endl;
  *stream << "                 f (float, -1.0 ~ 1.0) " << std::endl;
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  infile:" << std::endl;
  *stream << "       audio file                       [stdin]" <<std::endl;
  *stream << "  stdout:" << std::endl;
  *stream << "       waveform                 (  type)" << std::endl;
  *stream << "  notice:" << std::endl;
  *stream << "       input format is automatically detected from file extension" << std::endl;  // NOLINT
  *stream << "       default input format is WAV when reading from standard input" << std::endl;  // NOLINT
  *stream << "       use sox or ffmpeg to handle other formats" << std::endl;
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

class AudioReaderInterface {
 public:
  virtual ~AudioReaderInterface() = default;

  virtual bool Initialize(const char* input_file) = 0;

  virtual bool Initialize(const std::vector<char>& buffer) = 0;

  virtual void Finalize() = 0;

  virtual size_t GetTotalSamples() = 0;

  virtual bool Run() = 0;
};

template <typename T>
class AudioReader : public AudioReaderInterface {
 public:
  bool Run() override {
    const size_t num_samples(GetTotalSamples());
    std::vector<T> raw_data(num_samples);
    if (!Read(raw_data.data())) {
      return false;
    }
    if (!sptk::WriteStream(0, num_samples, raw_data, &std::cout, NULL)) {
      return false;
    }
    return true;
  }

 private:
  template <typename U>
  typename std::enable_if<std::is_same<U, std::int16_t>::value, bool>::type
  Read(U* data) {
    return ReadInt16(data);
  }

  template <typename U>
  typename std::enable_if<std::is_same<U, float>::value, bool>::type Read(
      U* data) {
    return ReadFloat32(data);
  }

  virtual bool ReadInt16(std::int16_t* data) = 0;

  virtual bool ReadFloat32(float* data) = 0;
};

template <typename T>
class WavReader final : public AudioReader<T> {
 public:
  WavReader() : wav_() {
  }

  ~WavReader() override {
    Finalize();
  }

  bool Initialize(const char* input_file) override {
    Finalize();
    return drwav_init_file(&wav_, input_file, NULL);
  }

  bool Initialize(const std::vector<char>& buffer) override {
    Finalize();
    return drwav_init_memory(&wav_, buffer.data(), buffer.size(), NULL);
  }

  void Finalize() override {
    drwav_uninit(&wav_);
  }

  size_t GetTotalSamples() override {
    return wav_.totalPCMFrameCount * wav_.channels;
  }

 private:
  bool ReadInt16(std::int16_t* data) override {
    if (NULL == data) {
      return false;
    }
    return wav_.totalPCMFrameCount ==
           drwav_read_pcm_frames_s16(&wav_, wav_.totalPCMFrameCount, data);
  }

  bool ReadFloat32(float* data) override {
    if (NULL == data) {
      return false;
    }
    return wav_.totalPCMFrameCount ==
           drwav_read_pcm_frames_f32(&wav_, wav_.totalPCMFrameCount, data);
  }

  drwav wav_;

  DISALLOW_COPY_AND_ASSIGN(WavReader<T>);
};

template <typename T>
class Mp3Reader final : public AudioReader<T> {
 public:
  Mp3Reader() : mp3_() {
  }

  ~Mp3Reader() override {
    Finalize();
  }

  bool Initialize(const char* input_file) override {
    Finalize();
    return drmp3_init_file(&mp3_, input_file, NULL);
  }

  bool Initialize(const std::vector<char>& buffer) override {
    Finalize();
    return drmp3_init_memory(&mp3_, buffer.data(), buffer.size(), NULL);
  }

  void Finalize() override {
    drmp3_uninit(&mp3_);
  }

  size_t GetTotalSamples() override {
    return GetTotalPCMFrameCount() * mp3_.channels;
  }

 private:
  bool ReadInt16(std::int16_t* data) override {
    if (NULL == data) {
      return false;
    }
    const drmp3_uint64 pcm_frame_count(GetTotalPCMFrameCount());
    return pcm_frame_count ==
           drmp3_read_pcm_frames_s16(&mp3_, pcm_frame_count, data);
  }

  bool ReadFloat32(float* data) override {
    if (NULL == data) {
      return false;
    }
    const drmp3_uint64 pcm_frame_count(GetTotalPCMFrameCount());
    return pcm_frame_count ==
           drmp3_read_pcm_frames_f32(&mp3_, pcm_frame_count, data);
  }

  size_t GetTotalPCMFrameCount() {
    drmp3_uint64 pcm_frame_count;
    if (!drmp3_get_mp3_and_pcm_frame_count(&mp3_, NULL, &pcm_frame_count)) {
      return 0;
    }
    pcm_frame_count -= (mp3_.delayInPCMFrames + mp3_.paddingInPCMFrames);
    return pcm_frame_count;
  }

  drmp3 mp3_;

  DISALLOW_COPY_AND_ASSIGN(Mp3Reader<T>);
};

template <typename T>
class FlacReader final : public AudioReader<T> {
 public:
  FlacReader() : flac_(NULL) {
  }

  ~FlacReader() override {
    Finalize();
  }

  bool Initialize(const char* input_file) override {
    Finalize();
    flac_ = drflac_open_file(input_file, NULL);
    return NULL != flac_;
  }

  bool Initialize(const std::vector<char>& buffer) override {
    Finalize();
    flac_ = drflac_open_memory(buffer.data(), buffer.size(), NULL);
    return NULL != flac_;
  }

  void Finalize() override {
    if (NULL != flac_) {
      drflac_close(flac_);
      flac_ = NULL;
    }
  }

  size_t GetTotalSamples() override {
    if (NULL == flac_) {
      return 0;
    }
    return flac_->totalPCMFrameCount * flac_->channels;
  }

 private:
  bool ReadInt16(std::int16_t* data) override {
    if (NULL == data || NULL == flac_) {
      return false;
    }
    return flac_->totalPCMFrameCount ==
           drflac_read_pcm_frames_s16(flac_, flac_->totalPCMFrameCount, data);
  }

  bool ReadFloat32(float* data) override {
    if (NULL == data || NULL == flac_) {
      return false;
    }
    return flac_->totalPCMFrameCount ==
           drflac_read_pcm_frames_f32(flac_, flac_->totalPCMFrameCount, data);
  }

  drflac* flac_;

  DISALLOW_COPY_AND_ASSIGN(FlacReader<T>);
};

template <typename T>
class OggReader final : public AudioReader<T> {
 public:
  OggReader() : vorbis_(NULL) {
  }

  ~OggReader() override {
    Finalize();
  }

  bool Initialize(const char* input_file) override {
    Finalize();
    vorbis_ = stb_vorbis_open_filename(input_file, NULL, NULL);
    return NULL != vorbis_;
  }

  bool Initialize(const std::vector<char>& buffer) override {
    Finalize();
    vorbis_ = stb_vorbis_open_memory(
        reinterpret_cast<const unsigned char*>(buffer.data()), buffer.size(),
        NULL, NULL);
    return NULL != vorbis_;
  }

  void Finalize() override {
    if (NULL != vorbis_) {
      stb_vorbis_close(vorbis_);
      vorbis_ = NULL;
    }
  }

  size_t GetTotalSamples() override {
    if (NULL == vorbis_) {
      return 0;
    }
    return stb_vorbis_stream_length_in_samples(vorbis_) * vorbis_->channels;
  }

 private:
  bool ReadInt16(std::int16_t* data) override {
    if (NULL == data || NULL == vorbis_) {
      return false;
    }
    const int num_shorts(GetTotalSamples());
    return num_shorts / vorbis_->channels ==
           stb_vorbis_get_samples_short_interleaved(vorbis_, vorbis_->channels,
                                                    data, num_shorts);
  }

  bool ReadFloat32(float* data) override {
    if (NULL == data || NULL == vorbis_) {
      return false;
    }
    const int num_floats(GetTotalSamples());
    return num_floats / vorbis_->channels ==
           stb_vorbis_get_samples_float_interleaved(vorbis_, vorbis_->channels,
                                                    data, num_floats);
  }

  stb_vorbis* vorbis_;

  DISALLOW_COPY_AND_ASSIGN(OggReader<T>);
};

class AudioReaderWrapper {
 public:
  AudioReaderWrapper(InputFormats input_format,
                     const std::string& output_data_type)
      : audio_reader_(NULL) {
    if (kWav == input_format && "s" == output_data_type) {
      audio_reader_ = new WavReader<std::int16_t>();
    } else if (kWav == input_format && "f" == output_data_type) {
      audio_reader_ = new WavReader<float>();
    } else if (kMp3 == input_format && "s" == output_data_type) {
      audio_reader_ = new Mp3Reader<std::int16_t>();
    } else if (kMp3 == input_format && "f" == output_data_type) {
      audio_reader_ = new Mp3Reader<float>();
    } else if (kFlac == input_format && "s" == output_data_type) {
      audio_reader_ = new FlacReader<std::int16_t>();
    } else if (kFlac == input_format && "f" == output_data_type) {
      audio_reader_ = new FlacReader<float>();
    } else if (kOgg == input_format && "s" == output_data_type) {
      audio_reader_ = new OggReader<std::int16_t>();
    } else if (kOgg == input_format && "f" == output_data_type) {
      audio_reader_ = new OggReader<float>();
    }
  }

  ~AudioReaderWrapper() {
    delete audio_reader_;
  }

  bool IsValid() const {
    return NULL != audio_reader_;
  }

  bool Run(const char* input_file) const {
    return (IsValid() && audio_reader_->Initialize(input_file) &&
            audio_reader_->Run());
  }

  bool Run(const std::vector<char>& buffer) const {
    return (IsValid() && audio_reader_->Initialize(buffer) &&
            audio_reader_->Run());
  }

 private:
  AudioReaderInterface* audio_reader_;

  DISALLOW_COPY_AND_ASSIGN(AudioReaderWrapper);
};

}  // namespace

/**
 * @a wav2raw [ @e option ] [ @e infile ]
 *
 * - @b -q @e int
 *   - input audio format
 *     \arg @c 0 WAV
 *     \arg @c 1 MP3
 *     \arg @c 2 FLAC
 *     \arg @c 3 OGG
 * - @b +type @e char
 *   - output data type
 *     \arg @c s short (-32768 ~ 32767)
 *     \arg @c f float (-1.0 ~ 1.0)
 * - @b infile @e str
 *   - audio file
 * - @b stdout
 *   - waveform
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  InputFormats input_format(kDefaultInputFormat);
  bool is_input_format_specified(false);
  std::string output_data_type(kDefaultOutputDataType);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "q:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'q': {
        const int min(0);
        const int max(static_cast<int>(kNumInputFormats) - 1);
        int tmp;
        if (!sptk::ConvertStringToInteger(optarg, &tmp) ||
            !sptk::IsInRange(tmp, min, max)) {
          std::ostringstream error_message;
          error_message << "The argument for the -q option must be an integer "
                        << "in the range of " << min << " to " << max;
          sptk::PrintErrorMessage("wav2raw", error_message);
          return 1;
        }
        input_format = static_cast<InputFormats>(tmp);
        is_input_format_specified = true;
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
      output_data_type = str.substr(1, std::string::npos);
    } else if (NULL == input_file) {
      input_file = arg;
    } else {
      std::ostringstream error_message;
      error_message << "Too many input files";
      sptk::PrintErrorMessage("wav2raw", error_message);
      return 1;
    }
  }

  if (!sptk::SetBinaryMode()) {
    std::ostringstream error_message;
    error_message << "Cannot set translation mode";
    sptk::PrintErrorMessage("wav2raw", error_message);
    return 1;
  }

  std::vector<char> buffer;
  if (input_file) {
    if (!is_input_format_specified) {
      // Determine input format from file extension.
      const char* dot_ext(std::strrchr(input_file, '.'));
      if (NULL != dot_ext && 1 < std::strlen(dot_ext)) {
        std::string ext(dot_ext + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](char c) { return std::tolower(c); });
        if (ext == "wav") {
          input_format = kWav;
        } else if (ext == "mp3") {
          input_format = kMp3;
        } else if (ext == "flac") {
          input_format = kFlac;
        } else if (ext == "ogg") {
          input_format = kOgg;
        } else {
          std::ostringstream error_message;
          error_message << "Unknown file extension: " << ext;
          sptk::PrintErrorMessage("wav2raw", error_message);
          return 1;
        }
      }
    }
  } else {
    // Read all data from stdin.
    buffer.assign((std::istreambuf_iterator<char>(std::cin)),
                  std::istreambuf_iterator<char>());
  }

  AudioReaderWrapper audio_reader(input_format, output_data_type);
  if (!audio_reader.IsValid()) {
    std::ostringstream error_message;
    error_message << "Unexpected input/output format";
    sptk::PrintErrorMessage("wav2raw", error_message);
    return 1;
  }
  if ((input_file && !audio_reader.Run(input_file)) ||
      (!input_file && !audio_reader.Run(buffer))) {
    std::ostringstream error_message;
    error_message << "Failed to convert";
    sptk::PrintErrorMessage("wav2raw", error_message);
    return 1;
  }

  return 0;
}
