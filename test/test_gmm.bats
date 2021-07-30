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

sptk3=tools/sptk/bin
sptk4=bin

setup() {
   mkdir -p tmp
}

teardown() {
   rm -rf tmp
}

@test "gmm: compatibility" {
   # Note that there is no compatibility without diagonal covariance.
   # This is because SPTK3 use previous mean instead of current mean
   # to update covariance.
   $sptk3/nrand -s 1 -l 256 | $sptk3/gmm -l 4 -m 4 -b 19 > tmp/1
   $sptk3/nrand -s 1 -l 256 | $sptk4/gmm -l 4 -k 4 -i 20 > tmp/2

   $sptk3/bcp +d -l 9 -s 0 -e 0 tmp/2 > tmp/2.w
   $sptk3/bcp +d -l 9 -s 1 tmp/2 > tmp/2.mv
   cat tmp/2.w tmp/2.mv > tmp/3
   run $sptk4/aeq tmp/1 tmp/3
   [ "$status" -eq 0 ]

   # MAP estimate.
   $sptk3/nrand -s 2 -l 256 | \
      $sptk3/gmm -l 4 -m 4 -b 19 -F tmp/1 -M 0.1 > tmp/4
   $sptk3/nrand -s 2 -l 256 | \
      $sptk4/gmm -l 4 -k 4 -i 20 -U tmp/2 -M 0.1 > tmp/5

   $sptk3/bcp +d -l 9 -s 0 -e 0 tmp/5 > tmp/5.w
   $sptk3/bcp +d -l 9 -s 1 tmp/5 > tmp/5.mv
   cat tmp/5.w tmp/5.mv > tmp/6
   run $sptk4/aeq tmp/4 tmp/6
   [ "$status" -eq 0 ]
}

@test "gmm: valgrind" {
   $sptk3/nrand -l 20 > tmp/1
   run valgrind $sptk4/gmm -l 2 -k 2 tmp/1
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
