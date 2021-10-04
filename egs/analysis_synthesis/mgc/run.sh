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

mkdir -p $dump

# Extract mel-cepstrum.
$sptk4/x2x +sd $data |
    $sptk4/frame -l $fl -p $fp |
    $sptk4/window -l $fl -L $nfft |
    $sptk4/mgcep -l $nfft -m $order -a $alpha > $dump/data.mgc

# Extract pitch.
$sptk4/x2x +sd $data |
    $sptk4/pitch -s $sr -p $fp -o 0 > $dump/data.pit

# Synthesis from extracted features.
$sptk4/excite -p $fp $dump/data.pit |
    $sptk4/mglsadf -p $fp -m $order -a $alpha -P 7 $dump/data.mgc |
    $sptk4/x2x +ds -r > $dump/data.syn.raw

# Fast-speaking voice.
$sptk4/excite -p $((fp/2)) $dump/data.pit |
    $sptk4/mglsadf -p $((fp/2)) -m $order -a $alpha -P 7 $dump/data.mgc |
    $sptk4/x2x +ds -r > $dump/data.fast.raw

# Slow-speaking voice.
$sptk4/excite -p $((fp*2)) $dump/data.pit |
    $sptk4/mglsadf -p $((fp*2)) -m $order -a $alpha -P 7 $dump/data.mgc |
    $sptk4/x2x +ds -r > $dump/data.slow.raw

# Hoarse voice.
l=$($sptk4/x2x +sa $data | wc -l)
$sptk4/mseq -l "$l" |
    $sptk4/mglsadf -p $fp -m $order -a $alpha -P 7 $dump/data.mgc |
    $sptk4/x2x +ds -r > $dump/data.hoarse.raw

# Robotic voice.
$sptk4/train -p 200 -l "$l" |
    $sptk4/mglsadf -p $fp -m $order -a $alpha -P 7 $dump/data.mgc |
    $sptk4/x2x +ds -r > $dump/data.robot.raw

# Child-like voice.
$sptk4/sopr -m 0.4 $dump/data.pit |
    $sptk4/excite -p $fp |
    $sptk4/mglsadf -p $fp -m $order -a 0.1 -P 7 $dump/data.mgc |
    $sptk4/x2x +ds -r > $dump/data.child.raw

# Deep voice.
$sptk4/sopr -m 2 $dump/data.pit |
    $sptk4/excite -p $fp |
    $sptk4/mglsadf -p $fp -m $order -a 0.6 -P 7 $dump/data.mgc |
    $sptk4/x2x +ds -r > $dump/data.deep.raw

echo "run.sh: successfully finished"
