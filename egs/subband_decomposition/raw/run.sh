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

nband=4   # Number of subbands
order=39  # Order of filter

mkdir -p $dump

# Analysis.
$sptk4/x2x +sd $data |
    $sptk4/pqmf -k $nband -m $order |
    $sptk4/decimate -l $nband -p $nband > $dump/data.ana.raw

# Synthesis.
$sptk4/interpolate -l $nband -p $nband < $dump/data.ana.raw |
    $sptk4/sopr -m $nband |
    $sptk4/ipqmf -k $nband -m $order |
    $sptk4/x2x +ds -r > $dump/data.syn.raw

echo "run.sh: successfully finished"
