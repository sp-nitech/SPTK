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

mkdir -p $dump

# Extract mel-cepstrum.
$sptk4/x2x +sd $data | \
   $sptk4/frame -l $fl -p $fp | \
   $sptk4/window -l $fl -L $nfft | \
   $sptk4/mgcep -l $nfft -m $order -a $alpha > $dump/data.mgc

# Extract pitch.
$sptk4/x2x +sd $data | \
   $sptk4/pitch -s $sr -p $fp -o 0 > $dump/data.pit

# Synthesis from extracted features.
cat $dump/data.pit | \
   $sptk4/excite -p $fp | \
   $sptk4/mglsadf -p $fp -m $order -a $alpha -P 7 $dump/data.mgc | \
   $sptk4/x2x +ds -r > $dump/data.syn.raw

# Fast-speaking voice.
cat $dump/data.pit | \
   $sptk4/excite -p $(($fp/2)) | \
   $sptk4/mglsadf -p $(($fp/2)) -m $order -a $alpha -P 7 $dump/data.mgc | \
   $sptk4/x2x +ds -r > $dump/data.fast.raw

# Slow-speaking voice.
cat $dump/data.pit | \
   $sptk4/excite -p $(($fp*2)) | \
   $sptk4/mglsadf -p $(($fp*2)) -m $order -a $alpha -P 7 $dump/data.mgc | \
   $sptk4/x2x +ds -r > $dump/data.slow.raw

# Hoarse voice.
l=$($sptk4/x2x +sa $data | wc -l)
$sptk4/mseq -l $l | \
   $sptk4/mglsadf -p $fp -m $order -a $alpha -P 7 $dump/data.mgc | \
   $sptk4/x2x +ds -r > $dump/data.hoarse.raw

# Robotic voice.
$sptk4/train -p 200 -l $l | \
   $sptk4/mglsadf -p $fp -m $order -a $alpha -P 7 $dump/data.mgc | \
   $sptk4/x2x +ds -r > $dump/data.robot.raw

# Child-like voice.
$sptk4/sopr -m 0.4 $dump/data.pit | \
   $sptk4/excite -p $fp | \
   $sptk4/mglsadf -p $fp -m $order -a 0.1 -P 7 $dump/data.mgc | \
   $sptk4/x2x +ds -r > $dump/data.child.raw

# Deep voice.
$sptk4/sopr -m 2 $dump/data.pit | \
   $sptk4/excite -p $fp | \
   $sptk4/mglsadf -p $fp -m $order -a 0.6 -P 7 $dump/data.mgc | \
   $sptk4/x2x +ds -r > $dump/data.deep.raw
