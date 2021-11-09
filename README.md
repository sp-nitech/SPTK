SPTK 4.0 (Under Construction)
=============================
The Speech Signal Processing Toolkit (SPTK) is a software for speech signal processing tools for UNIX environments.

[![](https://img.shields.io/badge/docs-latest-blue.svg)](https://sp-nitech.github.io/sptk/latest/)
[![](http://img.shields.io/badge/license-Apache%202.0-green.svg)](https://github.com/sp-nitech/SPTK/blob/master/LICENSE)
[![](https://github.com/sp-nitech/SPTK/workflows/build/badge.svg)](https://github.com/sp-nitech/SPTK/actions)


Documentation
-------------
See [this page](https://sp-nitech.github.io/sptk/latest/) for a reference manual.


Requirements
------------
- GCC 4.8+


Installation
------------
The latest release can be installed through Git:
```sh
git clone https://github.com/sp-nitech/SPTK.git
cd SPTK
make -j 4  # Please change the number of jobs depending on your environment.
```
Then the SPTK commands can be used by adding `SPTK/bin/` directory to the `PATH` environment variable.
If you would like to use a part of the SPTK library, please link the static library `SPTK/lib/libsptk.a`.


Examples
--------
The SPTK provides some examples.
Go to an example directory and execute `run.sh`, e.g.,
```sh
cd egs/analysis_synthesis/mgc
./run.sh
```

The below is a simple example that decreases the volume of input audio.
You may need to install `sox` command on your system.
```sh
sox -t wav input.wav -c 1 -t s16 -r 16000 - |
    x2x +sd | sopr -m 0.5 | x2x +ds -r |
    sox -c 1 -t s16 -r 16000 - -t wav output.wav
```

If you would like to draw figures, please prepare a python environment.
```sh
cd tools; make env; cd ..
. ./tools/venv/bin/activate
impulse -l 32 | gseries impulse.png
deactivate
```


Changes from SPTK3
------------------
- **Input and output types are changed to double from float**
- Drawing commands are implemented in Python
- No memory leaks
- New features:
  - Provide signal processing classes written in C++
  - Conversion from/to log area ratio (`lar2par` and `par2lar`)
  - Entropy calculation (`entropy`)
  - Huffman coding (`huffman`, `huffman_encode`, and `huffman_decode`)
  - Mel-cepstrum postfilter (`mcpf`)
  - Mel-filter-bank extraction (`fbank`)
  - Nonrecursive MLPG (`mlpg -R 1`)
  - Pitch extraction by DIO used in WORLD (`pitch -a 3`)
  - Scalar quantization (`quantize` and `dequantize`)
  - Stability check of LPC coefficients (`lpccheck`)
  - Subband decomposition (`pqmf` and `ipqmf`)
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
  - `dtw` -> `dtw` and `dtw_merge`
  - `mglsadf` -> `mglsadf` and `imglsadf`
  - `train` -> `train` and `mseq`
  - `ulaw` -> `ulaw` and `iulaw`
  - `vstat` -> `vstat` and `median`
- Renamed commands:
  - `c2ir` -> `c2mpir`
  - `mgclsp2sp` -> `mglsp2sp`


Relationship at a glance
------------------------
![diagram](asset/diagram.png?raw=true)


Who we are
----------
* **Keiichi Tokuda** - *Produce and Design* - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~tokuda/)
* **Keiichiro Oura** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~uratec/)
* **Takenori Yoshimura** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~takenori/)
* **Takato Fujimoto** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~taka19/)


Acknowledgements
----------------
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
* Satoshi Imai
* Shinji Sako
* Yoshihiko Nankaku


License
-------
This software is released under the Apache License 2.0.
