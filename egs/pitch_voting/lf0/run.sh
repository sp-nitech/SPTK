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

sr=16         # Sample rate in kHz
fp=$((sr*5))  # Frame shift  (16kHz x 5ms)

mkdir -p $dump

# Extract pitch.
for a in $(seq 0 3); do
    $sptk4/x2x +sd $data |
        $sptk4/pitch -s $sr -p $fp -o 2 -a "$a" > $dump/data.lf0."$a"
done

# Perform voting.
$sptk4/merge -l 1 -L 1 $dump/data.lf0.1 < $dump/data.lf0.0 |
    $sptk4/merge -l 2 -L 1 $dump/data.lf0.2 |
    $sptk4/merge -l 3 -L 1 $dump/data.lf0.3 |
    $sptk4/medfilt -l 4 -k 2 -magic -1e+10 -w 1 > $dump/data.lf0

# Draw pitch contours.
# shellcheck disable=SC1091
. ../../../tools/venv/bin/activate
n=$($sptk4/x2x +da $dump/data.lf0 | wc -l)
cat $dump/data.lf0.? $dump/data.lf0 |
    $sptk4/sopr -magic -1e+10 -EXP -MAGIC 0 |
    $sptk4/fdrw -n "$n" -g $dump/contour.png \
                -xname "Time [frame]" \
                -yname "Fundamental frequency [Hz]" \
                -names "RAPT,SWIPE,REAPER,WORLD,Voting"

echo "run.sh: successfully finished"
