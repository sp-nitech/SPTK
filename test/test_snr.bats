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

@test "snr: compatibility" {
   $sptk3/nrand -s 123 -l 50 > tmp/1
   $sptk3/nrand -s 321 -l 50 > tmp/2

   # SNR
   $sptk3/snr -o 2 tmp/1 tmp/2 > tmp/3
   $sptk4/snr -o 0 tmp/1 tmp/2 > tmp/4
   run $sptk4/aeq tmp/3 tmp/4
   [ "$status" -eq 0 ]

   # segmental SNR
   $sptk3/snr -o 3 -l 10 tmp/1 tmp/2 > tmp/5
   $sptk4/snr -o 1 -l 10 tmp/1 tmp/2 > tmp/6
   run $sptk4/aeq tmp/5 tmp/6
   [ "$status" -eq 0 ]

   # segmental SNR per frame
   $sptk3/snr -o 1 -l 10 tmp/1 tmp/2 | \
      grep \~ | tr -s ' ' | cut -d' ' -f 6 | $sptk3/x2x +ad > tmp/7
   $sptk4/snr -o 2 -l 10 tmp/1 tmp/2 > tmp/8
   run $sptk4/aeq tmp/7 tmp/8
   [ "$status" -eq 0 ]
}

@test "snr: valgrind" {
   $sptk3/nrand -s 123 -l 10 > tmp/1
   $sptk3/nrand -s 321 -l 10 > tmp/2
   run valgrind $sptk4/snr tmp/1 tmp/2
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
