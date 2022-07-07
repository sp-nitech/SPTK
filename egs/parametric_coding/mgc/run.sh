#!/bin/bash
# ------------------------------------------------------------------------ #
# Copyright 2021 SPTK Working Group                                        #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ------------------------------------------------------------------------ #

set -euo pipefail

sptk4=../../../bin
data=../../../asset/data.short
dump=dump

sr=16          # Sample rate in kHz
fl=$((sr*25))  # Frame length (16kHz x 25ms)
fp=$((sr*5))   # Frame shift  (16kHz x 5ms)
nfft=512       # FFT length
order=24       # Order of mel-cepstrum
alpha=0.42     # Alpha of mel-cepstrum
size=32        # Codebook size

mkdir -p $dump

# Extract mel-cepstrum.
$sptk4/x2x +sd $data |
    $sptk4/frame -l $fl -p $fp |
    $sptk4/window -l $fl -L $nfft |
    $sptk4/mgcep -l $nfft -m $order -a $alpha > $dump/data.mgc

# Extract log f0.
$sptk4/x2x +sd $data |
    $sptk4/pitch -s $sr -p $fp -o 2 > $dump/data.lf0

# Make codebook of mel-cepstrum.
$sptk4/lbg -m $order -e $size $dump/data.mgc > $dump/mgc.cb

# Make codebook of log f0.
$sptk4/sopr -magic -1e+10 $dump/data.lf0 |
    $sptk4/lbg -m 0 -e $size > $dump/lf0.cb

# Encode mel-cepstrum.
$sptk4/msvq -m $order -s $dump/mgc.cb < $dump/data.mgc > $dump/enc.mgc

# Encode log f0.
$sptk4/msvq -m 0 -s $dump/lf0.cb < $dump/data.lf0 > $dump/enc.lf0

# Encode voiced/unvoiced symbol.
$sptk4/sopr -magic -1.0E+10 -m 0.0 -a 1.0 -MAGIC 0 $dump/data.lf0 |
    $sptk4/x2x +di > $dump/enc.vuv

# Decode mel-cepstrum.
$sptk4/imsvq -m $order -s $dump/mgc.cb < $dump/enc.mgc > $dump/dec.mgc

# Decode log f0 with voiced/unvoiced symbol.
$sptk4/x2x +id $dump/enc.vuv > $dump/dec.vuv
$sptk4/imsvq -m 0 -s $dump/lf0.cb < $dump/enc.lf0 |
    $sptk4/vopr -n 0 -m $dump/dec.vuv |
    $sptk4/sopr -magic 0 -MAGIC -1e+10 > $dump/dec.lf0

# Synthesis from decoded features.
$sptk4/sopr -magic -1e+10 -m -1 -EXP -m $((sr*1000)) -MAGIC 0 $dump/dec.lf0 |
    $sptk4/excite -p $fp |
    $sptk4/mglsadf -p $fp -m $order -a $alpha -P 7 $dump/dec.mgc |
    $sptk4/x2x +ds -r > $dump/data.syn.raw

echo "run.sh: successfully finished"
