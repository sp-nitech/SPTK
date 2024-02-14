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
dump=dump

nband=4  # Number of subbands
order=39 # Order of filter
len=1024 # Number of FFT bins

mkdir -p $dump

# Get impulse response.
$sptk4/impulse -l $len |
    $sptk4/pqmf -k $nband -m $order -r > $dump/imp.pqmf

# Convert to frequency domain.
for k in $(seq $nband); do
    $sptk4/bcp -l $nband -s $((k - 1)) -e $((k - 1)) $dump/imp.pqmf |
        $sptk4/spec -l $len > $dump/"$k".spec
done

# Draw frequency response.
export VIRTUAL_ENV_DISABLE_PROMPT=1
# shellcheck disable=SC1091
. ../../../tools/venv/bin/activate
eval cat $dump/"{1..$nband}".spec |
    $sptk4/fdrw -n $((len / 2 + 1)) -g $dump/filter.png \
                -xname "Normalized frequency [cyc]" \
                -yname "Log amplitude [dB]" \
                -xscale 0.5

echo "run.sh: successfully finished"
