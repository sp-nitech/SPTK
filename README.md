SPTK
====
The Speech Signal Processing Toolkit (SPTK) is a software for speech signal processing tools.

- Older version: [SPTK3](https://sourceforge.net/projects/sp-tk/)
- PyTorch version: [diffsptk](https://github.com/sp-nitech/diffsptk)

[![](https://img.shields.io/badge/docs-latest-blue.svg)](https://sp-nitech.github.io/sptk/latest/)
[![](https://img.shields.io/badge/docs-stable-blue.svg)](https://sp-nitech.github.io/sptk/4.0/)
[![](http://img.shields.io/badge/license-Apache%202.0-green.svg)](https://github.com/sp-nitech/SPTK/blob/master/LICENSE)
[![](https://github.com/sp-nitech/SPTK/workflows/build/badge.svg)](https://github.com/sp-nitech/SPTK/actions)


Documentation
-------------
See [this page](https://sp-nitech.github.io/sptk/4.0/) for a reference manual.


Requirements
------------
- GCC 4.8.5+ / Clang 3.5.0+ / Visual Studio 2015+
- CMake 3.1+


Installation
------------

### Linux / macOS

<details><summary>expand</summary><div>

The latest release can be downloaded through Git.
The install procedure is as follows.
```sh
git clone https://github.com/sp-nitech/SPTK.git
cd SPTK
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=..  # Please change install directory.
make -j 4 install  # Please change the number of jobs depending on your environment.
```
Then the SPTK commands can be used by adding `bin/` directory to the `PATH` environment variable.
If you would like to use a part of the SPTK functions, please link the static library `lib/libsptk.a`.

</div></details>

### Windows

<details><summary>expand</summary><div>

You may need to add `cmake` and `MSBuild` to the `PATH` environment variable in advance.
Open Command Prompt and follow the below procedure:
```sh
cd /path/to/SPTK  # Please change here to your appropriate path.
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=..  # Please change install directory.
MSBuild -maxcpucount:4 /p:Configuration=Release INSTALL.vcxproj
```
You can compile the programs via GUI instead of running MSBuild.
Then the SPTK functions can be used by linking the static library `lib/sptk.lib`.

</div></details>


Demonstration
-------------
- [Twitter](https://twitter.com/SPTK_DSP)
- [Analysis-synthesis](https://colab.research.google.com/drive/1spX1v9mk6Itxa63R4wYwekzduvLeaUmE?usp=sharing) via mel-cepstrum
- [Parametric coding](https://colab.research.google.com/drive/1NeZxrWiNeixAWaux_HIBLbtaSeokUNiG?usp=sharing) via line spectral pairs


Examples
--------
The SPTK provides some examples.
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
cd tools; make venv; cd ..
. ./tools/venv/bin/activate
impulse -l 32 | gseries impulse.png
deactivate
```


Changes from SPTK3
------------------
- **Input and output types are changed to double from float**
- Signal processing classes are written in C++ instead of C
- Drawing commands are implemented in Python
- No memory leaks
- Thread-safe
- New features:
  - Conversion from/to log area ratio (`lar2par` and `par2lar`)
  - Entropy calculation (`entropy`)
  - Huffman coding (`huffman`, `huffman_encode`, and `huffman_decode`)
  - Magic number interpolation (`magic_intpl`)
  - Median filter (`medfilt`)
  - Mel-cepstrum postfilter (`mcpf`)
  - Mel-filter-bank extraction (`fbank`)
  - Nonrecursive MLPG (`mlpg -R 1`)
  - Pitch extraction by DIO used in WORLD (`pitch -a 3`)
  - Pole-zero plot (`gpolezero`)
  - Scalar quantization (`quantize` and `dequantize`)
  - Spectrogram plot (`gspecgram`)
  - Stability check of LPC coefficients (`lpccheck`)
  - Subband decomposition (`pqmf` and `ipqmf`)
  - Windows build support (**only static library**)
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


Overview
--------
![diagram](asset/diagram.png?raw=true)


Who we are
----------
* **Keiichi Tokuda** - *Produce and Design* - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~tokuda/)
* **Keiichiro Oura** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~uratec/)
* **Takenori Yoshimura** - *Main Maintainer* - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~takenori/)
* **Takato Fujimoto** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~taka19/)


Contributors to former versions of SPTK
---------------------------------------
* Akira Tamamori
* Cassia Valentini
* Chiyomi Miyajima
* Fernando Gil Resende Junior
* Gou Hirabayashi
* Heiga Zen
* Junichi Yamagishi
* Keiichi Tokuda
* Keiichiro Oura
* Kenji Chiba
* Masatsune Tamura
* Naohiro Isshiki
* Noboru Miyazaki
* Satoshi Imai
* Shinji Sako
* Tadashi Kitamura
* Takao Kobayashi
* Takashi Masuko
* Takashi Nose
* Takato Fujimoto
* Takayoshi Yoshimura
* Takenori Yoshimura
* Toru Takahashi
* Toshiaki Fukada
* Toshihiko Kato
* Toshio Kanno
* Yoshihiko Nankaku


License
-------
This software is released under the Apache License 2.0.
