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
nmix=2         # Number of mixtures

fp2=$((sr*10)) # Frame shift (target)
alpha2=0.3     # Alpha of mel-cepstrum (target)

mkdir -p $dump

# Make window.
echo -0.5 0.0 0.5 | $sptk4/x2x +ad > $dump/win.mgc

# Make source.
$sptk4/x2x +sd $data |
    $sptk4/frame -l $fl -p $fp |
    $sptk4/window -l $fl -L $nfft |
    $sptk4/mgcep -l $nfft -m $order -a $alpha > $dump/data.mgc.source
$sptk4/delta -m $order -D $dump/win.mgc $dump/data.mgc.source \
             > $dump/data.mgc.source.delta

# Make target.
$sptk4/x2x +sd $data |
    $sptk4/frame -l $fl -p $fp2 |
    $sptk4/window -l $fl -L $nfft |
    $sptk4/mgcep -l $nfft -m $order -a $alpha2 |
    $sptk4/delta -m $order -D $dump/win.mgc > $dump/data.mgc.target.delta

# Make oracle.
$sptk4/x2x +sd $data |
    $sptk4/frame -l $fl -p $fp |
    $sptk4/window -l $fl -L $nfft |
    $sptk4/mgcep -l $nfft -m $order -a $alpha2 > $dump/data.mgc.target

# Perform DTW.
dl=$((2*(order+1)))
$sptk4/dtw -l $dl -p 1 -S $dump/dtw.score $dump/data.mgc.target.delta \
           < $dump/data.mgc.source.delta > $dump/data.mgc.joint

# Train joint GMMs.
dl2=$((2*dl))
$sptk4/gmm -k $nmix -l $dl2 -B $dl $dl $dump/data.mgc.joint \
           > $dump/joint.gmm

# Perform GMM-based voice conversion.
$sptk4/vc -k $nmix -m $order -D $dump/win.mgc -f $dump/joint.gmm \
          < $dump/data.mgc.source.delta > $dump/data.mgc.convert

# Take the difference between source and converted one without c0.
$sptk4/ramp -l 1 > $dump/mask
$sptk4/step -l $order >> $dump/mask
$sptk4/vopr -s $dump/data.mgc.source < $dump/data.mgc.convert |
    $sptk4/vopr -n $order -m $dump/mask -q 1 > $dump/data.mgc.diff

# Generate waveform by differential filtering.
$sptk4/x2x +sd $data | \
    $sptk4/mglsadf -m $order -a $alpha -p $fp $dump/data.mgc.diff |
    $sptk4/x2x +ds -r > $dump/data.raw

# Calculate estimation error.
error=$($sptk4/rmse $dump/data.mgc.target $dump/data.mgc.convert | $sptk4/x2x +da)
echo "run.sh: RMSE = $error"

echo "run.sh: successfully finished"
