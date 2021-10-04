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

nbit=8  # Number of bits

mkdir -p $dump

# Quantize.
$sptk4/x2x +sd $data |
   $sptk4/ulaw |
   $sptk4/quantize -b $nbit > $dump/data.q

# Generate codebook.
b=$(echo 2 | $sptk4/x2x +ad | $sptk4/sopr -p $nbit | $sptk4/x2x +da)
$sptk4/x2x +id $dump/data.q |
   $sptk4/histogram -b "$b" -l 0 -u $((b-1)) |
   $sptk4/huffman > $dump/data.cb

# Encode and transmit.
$sptk4/huffman_encode $dump/data.cb $dump/data.q > $dump/data.enc

# Recieve and decode.
$sptk4/huffman_decode $dump/data.cb $dump/data.enc > $dump/data.dec

# Dequantize.
$sptk4/dequantize -b $nbit $dump/data.dec |
   $sptk4/iulaw |
   $sptk4/x2x +ds > $dump/data.raw

# Check size.
org_size=$(stat -c %s $data | $sptk4/x2x +ad |
                  $sptk4/sopr -m 8 | $sptk4/x2x +da)
ulaw_size=$(stat -c %s $dump/data.q | $sptk4/x2x +ad |
                   $sptk4/sopr -m 2 | $sptk4/x2x +da)
huff_size=$(stat -c %s $dump/data.enc)
echo "run.sh: original size = $org_size bits"
echo "run.sh: quantized size = $ulaw_size bits"
echo "run.sh: compressed size = $huff_size bits"

echo "run.sh: successfully finished"
