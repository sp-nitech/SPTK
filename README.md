SPTK 4.0 (Under Construction)
=============================
The Speech Signal Processing Toolkit (SPTK) is a software for speech signal processing tools for UNIX environments.

[![](https://img.shields.io/badge/docs-latest-blue.svg)](https://sp-nitech.github.io/sptk/latest/)
[![](http://img.shields.io/badge/license-BSD-green.svg)](https://github.com/sp-nitech/SPTK/blob/master/LICENSE)
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
make
```
Then the SPTK commands can be used by adding `SPTK/bin/` directory to the `PATH` environment variable.
If you would like to use a part of the SPTK library, please link the static library `SPTK/lib/libsptk.a`.


Changes from SPTK3
------------------
- Input and output types are changed to double from float
- Deprecated commands:
  - `acep`
  - `c2sp`
  - `gc2gc`
  - `gcep`
  - `glsadf`
  - `ivq`
  - `lmadf`
  - `lsp2sp`
  - `mcep`
  - `mlsadf`
  - `uels`
  - `vq`
- Split commands:
  - `dtw` -> `dtw` and `dtw_merge`
  - `mglsadf` -> `mglsadf` and `imglsadf`
  - `train` -> `train` and `mseq`
  - `ulaw` -> `ulaw` and `iulaw`
  - `vstat` -> `vstat` and `median`
- Integrated commands:
  - `agcep` and `amcep` -> `amgcep`


Authors
-------
* **Keiichi Tokuda** - *Produce and Design* - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~tokuda/)
* **Keiichiro Oura** - *Main Maintainer* - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~uratec/)
* **Takenori Yoshimura** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~takenori/)
* **Yoshihiko Nankaku** - [Nagoya Institute of Technology](http://www.sp.nitech.ac.jp/~nankaku/)


Contributors
------------
* **Cassia Valentini** - [The University of Edinburgh](http://www.cstr.ed.ac.uk/ssi/people/cvbotinh.html)
  * Calculated the coefficients of the 6th- and 7th-order modified Pade approximation.


License
-------
This software is released under the Modified BSD license - see LICENSE file for details.
