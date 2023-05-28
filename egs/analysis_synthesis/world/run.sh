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
fp=$((sr * 5)) # Frame shift (16kHz x 5ms)
ft=1024        # FFT length

mkdir -p $dump

# Extract pitch.
$sptk4/x2x +sd $data |
    $sptk4/pitch -s $sr -p $fp -o 0 -a 3 > $dump/data.pit

# Extract spectrum.
$sptk4/x2x +sd $data |
    $sptk4/pitch_spec -s $sr -p $fp -l $ft $dump/data.pit > $dump/data.sp

# Extract aperiodicity.
$sptk4/x2x +sd $data |
    $sptk4/ap -s $sr -p $fp -l $ft $dump/data.pit > $dump/data.ap

# Synthesis from extracted features.
$sptk4/world_synth -s $sr -p $fp -l $ft $dump/data.sp $dump/data.ap < $dump/data.pit |
    $sptk4/x2x +ds -r > $dump/data.syn.raw

echo "run.sh: successfully finished"
