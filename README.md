# SPTK

The Speech Signal Processing Toolkit (SPTK) is a comprehensive suite of software tools for speech signal processing.

- Older version: [SPTK3](https://sourceforge.net/projects/sp-tk/)
- PyTorch version: [diffsptk](https://github.com/sp-nitech/diffsptk)

[![](https://img.shields.io/badge/docs-latest-blue.svg)](https://sp-nitech.github.io/sptk/latest/)
[![](https://img.shields.io/badge/docs-stable-blue.svg)](https://sp-nitech.github.io/sptk/4.4/)
[![](https://img.shields.io/badge/license-Apache%202.0-green.svg)](https://github.com/sp-nitech/SPTK/blob/master/LICENSE)
[![](https://github.com/sp-nitech/SPTK/workflows/build/badge.svg)](https://github.com/sp-nitech/SPTK/actions)

## Overview

SPTK consists of over 100 independent commands for diverse speech signal processing tasks.
A key feature is that all commands communicate through standard input and output, enabling the creation of complex processing chains using pipes.

Below is a simple example of using SPTK commands in the terminal:

```
$ x2x +sd < data.raw | clip -l 32768 -u 32767 | x2x +da | less
```

As shown above, SPTK follows the Unix philosophy.
All data is handled in a raw, header-less format (typically 64-bit double-precision), allowing for seamless integration between tools.
Furthermore, all parameters are configured via command-line options, making the toolkit ideal for automation and scripting.

## Documentation

For more information, please refer to the following resources:

- [**Reference Manual**](https://sp-nitech.github.io/sptk/latest/) - Detailed command specifications and usage.
- [**Tutorial Slides**](https://speakerdeck.com/takenori/introduction-to-sptk-a-toolkit-for-speech-signal-processing) - A great starting point for beginners to understand the basics of SPTK.
- [**Interactive Tutorial**](https://colab.research.google.com/drive/1vmbIJQDhT5F26eCE5iYKQuEEGxYUv-uJ?usp=drive_link) (Google Colab) — The easiest way to try SPTK in your browser.
- [**Conference Paper**](https://www.isca-archive.org/ssw_2023/yoshimura23_ssw.html) - For technical details and background, please refer to our publication in the ISCA Archive.

Follow us on [**X (Twitter)**](https://twitter.com/SPTK_DSP) for the latest updates and demonstrations.

## Requirements

To build SPTK, you need a C++ compiler that supports **C++11** (or later) and CMake.

- **C++ Compiler**: GCC 4.8.5+ / Clang 3.5.0+ / Visual Studio 2015+
- **CMake**: 3.1+

## Installation

<details>
<summary>Linux / macOS</summary>

You can download and build the latest version of SPTK from source:

```sh
# Clone the repository
$ git clone https://github.com/sp-nitech/SPTK.git
$ cd SPTK

# Build the toolkit
$ make
```

To use SPTK commands from any directory, add the `SPTK/bin/` directory to your `PATH`.
For example, in your `.bashrc` or `.zshrc`:

```
export PATH="$PATH:/path/to/SPTK/bin"
```

If you wish to integrate SPTK functions into your own C++ projects, link against the static library `lib/libsptk.a`.

</details>

<details>
<summary>Windows</summary>

Before proceeding, ensure that `cmake` and `MSBuild` are added to your `PATH` environment variable.
You can build SPTK by running `make.bat` or by following these steps in the Command Prompt:

```bat
# Navigate to the SPTK directory
$ cd C:\path\to\SPTK

# Create a build directory
$ mkdir build
$ cd build

# Generate project files and build
$ cmake .. -DCMAKE_INSTALL_PREFIX=..
$ MSBuild /p:Configuration=Release INSTALL.vcxproj
```

Alternatively, you can open the generated `SPTK.sln` file in the build directory and compile using the Visual Studio GUI.

To use SPTK functions in your Windows projects, link against the static library `lib/sptk.lib`.

</details>

## Examples

### Simple Audio Processing

The following pipeline demonstrates how to decrease the volume of `input.wav` by half:

```sh
$ wav2raw +s input.wav | x2x +sd | sopr -m 0.5 | x2x +ds -r | raw2wav +s -s 16 > output.wav
```

### Running Provided Examples

SPTK includes various example scripts. To run them, navigate to an example directory and execute `run.sh`:

```sh
$ cd egs/analysis_synthesis/mgc
$ ./run.sh
```

### Drawing Figures

To visualize data, you can use the built-in Python environment for plotting:

```sh
# Set up a virtual environment (one-time)
$ cd tools; make venv PYTHON_VERSION=3.8; cd ..

# Generate a figure
$ . ./tools/venv/bin/activate
$ impulse -l 32 | gseries impulse.png
$ deactivate
```

## Changes from SPTK3

### Core Improvements

- **Enhanced Precision**: The default data type has been upgraded from 4-byte `float` to 8-byte **`double`** (64-bit).
- **C++ Engine**: The core signal processing logic is now implemented in **C++** (formerly C), ensuring better maintainability.
- **Modern Plotting**: Drawing commands have been migrated to **Python**, offering greater flexibility and modern visualization options.
- **Thread-Safety**: The library is now **thread-safe**, making it compatible with multi-threaded applications and parallel processing.
- **Cross-Platform Support**: Added official support for **Windows** (in addition to Linux and macOS).

### Command Updates

<details><summary>Obsoleted & Integrated</summary>

- `acep`, `agcep`, `amcep` → `amgcep`
- `bell`
- `c2sp` → `mgc2sp`
- `cat2`, `echo2`
- `da`
- `ds`, `us`, `us16`, `uscd` → `sox` or `ffmpeg`
- `fig`
- `gc2gc` → `mgc2mgc`
- `gcep`, `mcep`, `uels` → `mgcep`
- `glsadf`, `lmadf`, `mlsadf` → `mglsadf`
- `ivq`, `vq` → `imsvq`, `msvq`
- `lsp2sp` → `mglsp2sp`
- `mgc2mgclsp`, `mgclsp2mgc`
- `psgr`, `xgr`
- `wavjoin`, `wavsplit`

</details>

<details><summary>Separated & Renamed</summary>

- `c2ir` → `c2mpir` & `mpir2c`
- `dtw` → `dtw` & `dtw_merge`
- `mgclsp2sp` → `mglsp2sp`
- `mglsadf` → `mglsadf` & `imglsadf`
- `train` → `train` & `mseq`
- `ulaw` → `ulaw` & `iulaw`
- `vstat` → `vstat` & `median`

</details>

## Contributors

- **Keiichi Tokuda** (*Project Design*) - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~tokuda/)
- **Keiichiro Oura** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~uratec/)
- **Takenori Yoshimura** (*Lead Maintainer*) - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~takenori/)
- **Takato Fujimoto** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~taka19/)

## Contributors to Former Versions

We would like to express our gratitude to all the contributors who have supported SPTK's development over the years:

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

## Third-party Software Licenses

SPTK incorporates the following third-party libraries:

| Category | Library | License |
| :--- | :--- | :--- |
| **Pitch Extraction** | [Snack](https://github.com/scottypitcher/tcl-snack) | Tcl/Tk License |
| | [SWIPE'](https://github.com/kylebgorman/swipe) | MIT License |
| | [REAPER](https://github.com/google/REAPER) | Apache License 2.0 |
| **WORLD Analysis-Synthesis** | [WORLD](https://github.com/mmorise/World) | 3-Clause BSD License |
| **Audio Format Conversion** | [dr_libs](https://github.com/mackron/dr_libs) | Public Domain / MIT License |
| | [stb](https://github.com/nothings/stb) | Public Domain / MIT License |
| **Command-line Parser** | [ya_getopt](https://github.com/kubo/ya_getopt) | 2-Clause BSD License |

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
