#!/usr/bin/env bats
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

sptk3=tools/sptk/bin
sptk4=bin

setup() {
   mkdir -p tmp
}

teardown() {
   rm -rf tmp
}

@test "huffman_encode: reversibility" {
   $sptk3/nrand -l 8 | $sptk4/huffman > tmp/1
   $sptk3/ramp -s 7 -t -1 -l 8 > tmp/2
   $sptk3/x2x +di tmp/2 | $sptk4/huffman_encode tmp/1 | \
      $sptk4/huffman_decode tmp/1 | $sptk3/x2x +id > tmp/3
   run $sptk4/aeq tmp/2 tmp/3
   [ "$status" -eq 0 ]
}

@test "huffman_encode: valgrind" {
   $sptk3/nrand -l 8 | $sptk4/huffman > tmp/1
   $sptk3/ramp -l 8 | $sptk3/x2x +di > tmp/2
   run valgrind $sptk4/huffman_encode tmp/1 tmp/2
  [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
