# SPTK

The Speech Signal Processing Toolkit (SPTK) is a software for speech signal processing tools.

- Older version: [SPTK3](https://sourceforge.net/projects/sp-tk/)
- PyTorch version: [diffsptk](https://github.com/sp-nitech/diffsptk)

[![](https://img.shields.io/badge/docs-latest-blue.svg)](https://sp-nitech.github.io/sptk/latest/)
[![](https://img.shields.io/badge/docs-stable-blue.svg)](https://sp-nitech.github.io/sptk/4.3/)
[![](https://img.shields.io/badge/license-Apache%202.0-green.svg)](https://github.com/sp-nitech/SPTK/blob/master/LICENSE)
[![](https://github.com/sp-nitech/SPTK/workflows/build/badge.svg)](https://github.com/sp-nitech/SPTK/actions)

## What is SPTK?

- SPTK consists of over 100 commands for speech signal processing.
- The data format used in SPTK is raw header-less, i.e., there is no specific structure.
  Thanks to the data format, we can check file contents immediately on CUI.
  ```sh
  dmp +s data.raw
  ```
- The data used in the commands is passed through standard input/output.
  We can chain multiple processes using pipes.
  ```sh
  x2x +sd < data.raw | clip | x2x +da | less
  ```
- The data type is basically little-endian double 8 bytes.
- The commands do not require interactive user inputs.
  Parameters are set via command line options beforehand.
  ```sh
  impulse -l 4 | sopr -m 10 | x2x +da
  ```

## Documentation

- See [this page](https://sp-nitech.github.io/sptk/latest/) for a reference manual.
- Our [paper](https://www.isca-speech.org/archive/ssw_2023/yoshimura23_ssw.html) is available on the ISCA Archive.

## Requirements

- GCC 4.8.5+ / Clang 3.5.0+ / Visual Studio 2015+
- CMake 3.1+

## Installation

### Linux / macOS

<details><summary>expand</summary><div>

The latest release can be downloaded through Git.
The install procedure is as follows.

```sh
git clone https://github.com/sp-nitech/SPTK.git
cd SPTK
make
```

Then the SPTK commands can be used by adding `bin/` directory to the `PATH` environment variable.
If you would like to use a part of the SPTK functions, please link the static library `lib/libsptk.a`.

</div></details>

### Windows

<details><summary>expand</summary><div>

You may need to add `cmake` and `MSBuild` to the `PATH` environment variable in advance.
Please run `make.bat` or open Command Prompt and follow the below procedure:

```sh
cd /path/to/SPTK  # Please change here to your appropriate path.
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=..  # Please change install directory.
MSBuild /p:Configuration=Release INSTALL.vcxproj
```

You can compile SPTK via GUI instead of running MSBuild by opening the generated project file.
Then the SPTK functions can be used by linking the static library `lib/sptk.lib`.

</div></details>

## Demonstration

- [Twitter](https://twitter.com/SPTK_DSP)
- [Analysis-synthesis](https://colab.research.google.com/drive/1spX1v9mk6Itxa63R4wYwekzduvLeaUmE?usp=sharing) via mel-cepstrum
- [Parametric coding](https://colab.research.google.com/drive/1NeZxrWiNeixAWaux_HIBLbtaSeokUNiG?usp=sharing) via line spectral pairs

## Examples

SPTK provides some examples.
Go to an example directory and execute `run.sh`, e.g.,

```sh
cd egs/analysis_synthesis/mgc
./run.sh
```

The below is a simple example that decreases the volume of input audio in `input.wav`.
You may need to install `sox` command on your system.

```sh
sox -t wav input.wav -c 1 -t s16 -r 16000 - |
    x2x +sd | sopr -m 0.5 | x2x +ds -r |
    sox -c 1 -t s16 -r 16000 - -t wav output.wav
```

If you would like to draw figures, please prepare a python environment.

```sh
cd tools; make venv PYTHON_VERSION=3.8; cd ..
. ./tools/venv/bin/activate
impulse -l 32 | gseries impulse.png
deactivate
```

## Changes from SPTK3

- **Input and output types are changed to double from float**
- Signal processing classes are written in C++ instead of C
- Drawing commands are implemented in Python
- Some option names
- No memory leaks
- Thread-safe
- New main features:
  - Aperiodicity extraction (`ap`)
  - Dynamic range compression (`drc`)
  - Magic number interpolation (`magic_intpl`)
  - Median filter (`medfilt`)
  - Mel-filter-bank extraction (`fbank`)
  - Nonrecursive MLPG (`mlpg -R 1`)
  - Pitch adaptive spectrum estimation (`pitch_spec`)
  - Pitch extraction used in WORLD (`pitch -a 3` and `pitch -a 4`)
  - PLP extraction (`plp`)
  - Sinusoidal generation from pitch (`pitch2sin`)
  - Subband decomposition (`pqmf` and `ipqmf`)
  - WORLD synthesis (`world_synth`)
  - Windows build support
- Obsoleted commands:
  - `acep`, `agcep`, and `amcep` -> `amgcep`
  - `bell`
  - `c2sp` -> `mgc2sp`
  - `cat2` and `echo2`
  - `da`
  - `ds`, `us`, `us16`, and `uscd` -> `sox`
  - `fig`
  - `gc2gc` -> `mgc2mgc`
  - `gcep`, `mcep`, and `uels` -> `mgcep`
  - `glsadf`, `lmadf`, and `mlsadf` -> `mglsadf`
  - `ivq` and `vq` -> `imsvq` and `msvq`
  - `lsp2sp` -> `mglsp2sp`
  - `mgc2mgclsp` and `mgclsp2mgc`
  - `psgr` and `xgr`
  - `raw2wav`, `wav2raw`, `wavjoin`, and `wavsplit` -> `sox`
- Separated commands:
  - `c2ir` -> `c2mpir` and `mpir2c`
  - `dtw` -> `dtw` and `dtw_merge`
  - `mglsadf` -> `mglsadf` and `imglsadf`
  - `train` -> `train` and `mseq`
  - `ulaw` -> `ulaw` and `iulaw`
  - `vstat` -> `vstat` and `median`
- Renamed commands:
  - `mgclsp2sp` -> `mglsp2sp`

## Who we are

- **Keiichi Tokuda** - *Produce and Design* - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~tokuda/)
- **Keiichiro Oura** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~uratec/)
- **Takenori Yoshimura** - *Main Maintainer* - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~takenori/)
- **Takato Fujimoto** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~taka19/)

## Contributors to former versions of SPTK

- Akira Tamamori
- Cassia Valentini
- Chiyomi Miyajima
- Fernando Gil Resende Junior
- Gou Hirabayashi
- Heiga Zen
- Junichi Yamagishi
- Kazuhito Koishida
- Keiichi Tokuda
- Keiichiro Oura
- Kenji Chiba
- Masatsune Tamura
- Naohiro Isshiki
- Noboru Miyazaki
- Satoshi Imai
- Shinji Sako
- Tadashi Kitamura
- Takao Kobayashi
- Takashi Masuko
- Takashi Nose
- Takato Fujimoto
- Takayoshi Yoshimura
- Takenori Yoshimura
- Toru Takahashi
- Toshiaki Fukada
- Toshihiko Kato
- Toshio Kanno
- Yoshihiko Nankaku

## License

This software is released under the Apache License 2.0.

## Citation

```bibtex
@InProceedings{sp-nitech2023sptk,
  author = {Takenori Yoshimura and Takato Fujimoto and Keiichiro Oura and Keiichi Tokuda},
  title = {{SPTK4}: An open-source software toolkit for speech signal processing},
  booktitle = {12th ISCA Speech Synthesis Workshop (SSW 2023)},
  pages = {211--217},
  year = {2023},
}
```
