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
order=24       # Order of LPC

mkdir -p $dump

# Extract LPC.
$sptk4/x2x +sd $data |
    $sptk4/frame -l $fl -p $fp |
    $sptk4/window -l $fl |
    $sptk4/lpc -l $fl -m $order > $dump/data.lpc

# Extract pitch.
$sptk4/x2x +sd $data |
    $sptk4/pitch -p $fp -o 0 > $dump/data.pitch

# Synthesis from extracted features.
$sptk4/excite -p $fp $dump/data.pitch |
    $sptk4/poledf -p $fp -m $order $dump/data.lpc |
    $sptk4/x2x +ds -r > $dump/data.syn.raw

echo "run.sh: successfully finished"
