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
#                1996-2021  Nagoya Institute of Technology          #
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
nmix=2         # Number of mixtures

fp2=$(($sr*10))  # Frame shift (target)
alpha2=0.3       # Alpha of mel-cepstrum (target)

mkdir -p $dump

# Make window.
echo -0.5 0.0 0.5 | $sptk4/x2x +ad > $dump/win.mgc

# Make source.
$sptk4/x2x +sd $data | \
   $sptk4/frame -l $fl -p $fp | \
   $sptk4/window -l $fl -L $nfft | \
   $sptk4/mgcep -l $nfft -m $order -a $alpha > $dump/data.mgc.source
$sptk4/delta -m $order -D $dump/win.mgc $dump/data.mgc.source \
             > $dump/data.mgc.source.delta

# Make target.
$sptk4/x2x +sd $data | \
   $sptk4/frame -l $fl -p $fp2 | \
   $sptk4/window -l $fl -L $nfft | \
   $sptk4/mgcep -l $nfft -m $order -a $alpha2 | \
   $sptk4/delta -m $order -D $dump/win.mgc > $dump/data.mgc.target.delta

# Make oracle.
$sptk4/x2x +sd $data | \
   $sptk4/frame -l $fl -p $fp | \
   $sptk4/window -l $fl -L $nfft | \
   $sptk4/mgcep -l $nfft -m $order -a $alpha2 > $dump/data.mgc.target

# Perform DTW.
dl=$((2*($order+1)))
$sptk4/dtw -l $dl -p 5 -S $dump/dtw.score $dump/data.mgc.target.delta \
           < $dump/data.mgc.source.delta > $dump/data.mgc.joint

# Train joint GMMs.
dl2=$((2*$dl))
$sptk4/gmm -k $nmix -l $dl2 -B $dl $dl $dump/data.mgc.joint \
           > $dump/joint.gmm

# Perform GMM-based voice conversion.
$sptk4/vc -k $nmix -m $order -D $dump/win.mgc -f $dump/joint.gmm \
          < $dump/data.mgc.source.delta > $dump/data.mgc.convert

# Take the difference between source and converted one without c0.
$sptk4/ramp -l 1 > $dump/mask
$sptk4/step -l $order >> $dump/mask
$sptk4/vopr -s $dump/data.mgc.source < $dump/data.mgc.convert | \
   $sptk4/vopr -n $order -m $dump/mask -q 1 > $dump/data.mgc.diff

# Generate waveform by differential filtering.
$sptk4/x2x +sd $data | \
   $sptk4/mglsadf -m $order -a $alpha -p $fp $dump/data.mgc.diff | \
   $sptk4/x2x +ds -r > $dump/data.raw

# Calculate estimation error.
error=$($sptk4/rmse $dump/data.mgc.target $dump/data.mgc.convert | $sptk4/x2x +da)
echo "RMSE: $error"
