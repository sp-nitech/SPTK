#!/bin/bash
# ----------------------------------------------------------------- #
#             The Speech Signal Processing Toolkit (SPTK)           #
#             developed by SPTK Working Group                       #
#             http://sp-tk.sourceforge.net/                         #
# ----------------------------------------------------------------- #
#                                                                   #
#  Copyright (c) 1984-2007  Tokyo Institute of Technology           #
#                           Interdisciplinary Graduate School of    #
#                           Science and Engineering                 #
#                                                                   #
#                1996-2020  Nagoya Institute of Technology          #
#                           Department of Computer Science          #
#                                                                   #
# All rights reserved.                                              #
#                                                                   #
# Redistribution and use in source and binary forms, with or        #
# without modification, are permitted provided that the following   #
# conditions are met:                                               #
#                                                                   #
# - Redistributions of source code must retain the above copyright  #
#   notice, this list of conditions and the following disclaimer.   #
# - Redistributions in binary form must reproduce the above         #
#   copyright notice, this list of conditions and the following     #
#   disclaimer in the documentation and/or other materials provided #
#   with the distribution.                                          #
# - Neither the name of the SPTK working group nor the names of its #
#   contributors may be used to endorse or promote products derived #
#   from this software without specific prior written permission.   #
#                                                                   #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            #
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       #
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          #
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          #
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS #
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          #
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   #
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     #
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON #
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   #
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    #
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           #
# POSSIBILITY OF SUCH DAMAGE.                                       #
# ----------------------------------------------------------------- #

set -euo pipefail

sptk4=../../../bin
data=../../../asset/data.short
dump=dump

sr=16          # Sample rate in kHz
fl=$(($sr*25)) # Frame length (16kHz x 25ms)
fp=$(($sr*5))  # Frame shift  (16kHz x 5ms)
nfft=512       # FFT length
order=24       # Order of mel-cepstrum
alpha=0.42     # Alpha of mel-cepstrum
size=32        # Codebook size

mkdir -p $dump

# Extract mel-cepstrum.
$sptk4/x2x +sd $data | \
   $sptk4/frame -l $fl -p $fp | \
   $sptk4/window -l $fl -L $nfft | \
   $sptk4/mgcep -l $nfft -m $order -a $alpha > $dump/data.mgc

# Extract log f0.
$sptk4/x2x +sd $data | \
   $sptk4/pitch -p $fp -o 2 > $dump/data.lf0

# Make codebook of mel-cepstrum.
$sptk4/lbg -m $order -e $size $dump/data.mgc > $dump/mgc.cb

# Make codebook of log f0.
$sptk4/sopr -magic -1e+10 $dump/data.lf0 | \
   $sptk4/lbg -m 0 -e $size > $dump/lf0.cb

# Encode mel-cepstrum.
$sptk4/msvq -m $order -s $dump/mgc.cb < $dump/data.mgc > $dump/enc.mgc

# Encode log f0.
$sptk4/msvq -m 0 -s $dump/lf0.cb < $dump/data.lf0 > $dump/enc.lf0

# Encode voiced/unvoiced symbol.
$sptk4/sopr -magic -1.0E+10 -m 0.0 -a 1.0 -MAGIC 0 $dump/data.lf0 | \
   $sptk4/x2x +di > $dump/enc.vuv

# Decode mel-cepstrum.
$sptk4/imsvq -m $order -s $dump/mgc.cb < $dump/enc.mgc > $dump/dec.mgc

# Decode log f0 with voiced/unvoiced symbol.
$sptk4/x2x +id $dump/enc.vuv > $dump/dec.vuv
$sptk4/imsvq -m 0 -s $dump/lf0.cb < $dump/enc.lf0 | \
   $sptk4/vopr -n 0 -m $dump/dec.vuv | \
   $sptk4/sopr -magic 0 -MAGIC -1e+10 > $dump/dec.lf0

# Synthesis from decoded features.
$sptk4/sopr -magic -1e+10 -m -1 -EXP -m $(($sr*1000)) -MAGIC 0 $dump/dec.lf0 | \
   $sptk4/excite -p $fp | \
   $sptk4/mglsadf -p $fp -m $order -a $alpha -P 7 $dump/dec.mgc | \
   $sptk4/x2x +ds -r > $dump/data.syn.raw
