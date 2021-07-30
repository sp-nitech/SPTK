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

   # Make training data.
   $sptk3/nrand -s 1 -l 300 -m 1 -v 0.2 | \
      $sptk3/delta -d -0.5 0 0.5 -l 3 > tmp/0.s
   $sptk3/nrand -s 2 -l 200 -m -1 -v 0.1 | \
      $sptk3/delta -d -0.5 0 0.5 -l 2 > tmp/0.t
   $sptk3/merge +d tmp/0.s tmp/0.t -L 6 -l 4 > tmp/0
}

teardown() {
   rm -rf tmp
}

@test "vc: compatibility" {
   # Make GMM.
   $sptk3/gmm tmp/0 -l 10 -B 10 -m 4 > tmp/1

   # Rearrange the GMM to ensure the compatibility with SPTK4.
   $sptk3/bcut +d -e 3 tmp/1 > tmp/1.w
   $sptk3/bcut +d -s 4 tmp/1 | $sptk3/sopr -magic 0 > tmp/1.mv
   $sptk3/merge +d tmp/1.w tmp/1.mv -L 1 -l 20 > tmp/2

   # Make source.
   $sptk3/nrand -s 3 -l 100 -m 1 -v 0.2 > tmp/3

   # Make target.
   $sptk3/vc tmp/1 -l 3 -L 2 -m 4 -d -0.5 0 0.5 < tmp/3 > tmp/4
   $sptk3/delta -d -0.5 0 0.5 -l 3 tmp/3 | \
      $sptk4/vc tmp/2 -l 3 -L 2 -k 4 -d -0.5 0 0.5 > tmp/5
   run $sptk4/aeq tmp/4 tmp/5
   [ "$status" -eq 0 ]
}

@test "vc: block covariance" {
   # Make GMM.
   $sptk4/gmm tmp/0 -l 10 -B 6 4 -k 4 -f > tmp/1
   $sptk4/gmm tmp/0 -l 10 -k 4 > tmp/2

   # Make source.
   $sptk3/nrand -s 3 -l 30 -m 1 -v 0.2 | \
      $sptk3/delta -d -0.5 0 0.5 -l 3 > tmp/3

   # Make target.
   $sptk4/vc tmp/1 -l 3 -L 2 -k 4 -d -0.5 0 0.5 -f < tmp/3 > tmp/4
   $sptk4/vc tmp/2 -l 3 -L 2 -k 4 -d -0.5 0 0.5 < tmp/3 > tmp/5
   run $sptk4/aeq tmp/4 tmp/5 -e 1 -t 1
   [ "$status" -eq 0 ]
}

@test "vc: valgrind" {
   $sptk4/gmm tmp/0 -l 10 -k 2 > tmp/1
   $sptk3/nrand -l 20 | $sptk3/delta -d -0.5 0 0.5 -l 3 > tmp/2
   run valgrind $sptk4/vc tmp/1 tmp/2 -l 3 -L 2 -k 2 -d -0.5 0 0.5
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
