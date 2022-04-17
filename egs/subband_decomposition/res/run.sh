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
nband=8        # Number of subbands
fo=79          # Order of filter

mkdir -p $dump

# Extract mel-cepstrum.
$sptk4/x2x +sd $data |
    $sptk4/frame -l $fl -p $fp |
    $sptk4/window -l $fl -L $nfft |
    $sptk4/mgcep -l $nfft -m $order -a $alpha > $dump/data.mgc

# Inverse filtering.
$sptk4/x2x +sd $data |
    $sptk4/imglsadf -p $fp -m $order -a $alpha -P 7 $dump/data.mgc > $dump/data.res

# Analysis.
$sptk4/pqmf -k $nband -m $fo $dump/data.res |
    $sptk4/decimate -l $nband -p $nband > $dump/data.ana.res

# Synthesis.
$sptk4/interpolate -l $nband -p $nband < $dump/data.ana.res |
    $sptk4/sopr -m $nband |
    $sptk4/ipqmf -k $nband -m $fo > $dump/data.syn.res

# Filtering.
$sptk4/mglsadf -p $fp -m $order -a $alpha -P 7 $dump/data.mgc < $dump/data.syn.res |
    $sptk4/x2x +ds -r > $dump/data.syn.raw

echo "run.sh: successfully finished"
