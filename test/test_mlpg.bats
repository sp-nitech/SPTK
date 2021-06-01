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

@test "mlpg: compatibility" {
   # Make mean.
   $sptk3/nrand -s 1 -l 200 > tmp/1
   # Make variance.
   $sptk3/nrand -s 2 -l 200 | $sptk3/sopr -ABS -m 0.01 > tmp/2
   # Make pdf.
   $sptk3/merge +d -l 10 -L 10 tmp/1 tmp/2 > tmp/3

   # Perform MLPG.
   $sptk3/mlpg -l 5 -r 1 1 -s 15 tmp/3 > tmp/4
   $sptk4/mlpg -l 5 -r 1 -s 15 tmp/3 > tmp/5
   run $sptk4/aeq tmp/4 tmp/5
   [ "$status" -eq 0 ]

   # Remove zeros because SPTK3 does not consider delay.
   $sptk3/mlpg -l 5 -d -0.5 0 0.5 tmp/3 | $sptk3/x2x +da | grep -v ^0$ | \
      $sptk3/x2x +ad > tmp/6
   $sptk4/mlpg -l 5 -d -0.5 0 0.5 tmp/3 > tmp/7
   run $sptk4/aeq -t 1e-5 -L -e 1 tmp/6 tmp/7
   [ "$status" -eq 0 ]

   # Check consistency between recursive and non-recursive mode.
   $sptk4/mlpg -l 5 -d -0.5 0 0.5 -R 1 tmp/3 > tmp/8
   run $sptk4/aeq tmp/7 tmp/8
   [ "$status" -eq 0 ]
}

@test "mlpg: valgrind" {
   $sptk3/nrand -l 20 | $sptk3/sopr -ABS > tmp/1
   run valgrind $sptk4/mlpg -l 2 -R 0 tmp/1
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
   run valgrind $sptk4/mlpg -l 2 -R 1 tmp/1
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
