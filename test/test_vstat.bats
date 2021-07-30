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

@test "vstat: compatibility" {
   $sptk3/nrand -l 100 > tmp/0

   ary1=("-o 0" "-o 1" "-o 2" "-o 2 -d" "-o 2 -r" "-o 2 -i")
   ary2=("-o 0" "-o 1" "-o 2" "-o 2 -d" "-o 4"    "-o 5")
   for i in $(seq 0 $((${#ary1[@]} - 1))); do
      $sptk3/vstat -l 2 tmp/0 ${ary1[$i]} > tmp/1
      $sptk4/vstat -l 2 tmp/0 ${ary2[$i]} > tmp/2
      run $sptk4/aeq tmp/1 tmp/2
      [ "$status" -eq 0 ]
   done

   # lower and upper bounds
   $sptk3/vstat -l 2 tmp/0 -o 3 -c 90 > tmp/1
   $sptk4/vstat -l 2 tmp/0 -o 6 -c 90 > tmp/2
   $sptk3/bcut +d tmp/2 -s 0 -e 1 > tmp/2_0
   $sptk3/bcut +d tmp/2 -s 2 -e 3 > tmp/2_2
   $sptk3/bcut +d tmp/2 -s 4 -e 5 > tmp/2_1
   cat tmp/2_0 tmp/2_1 tmp/2_2 > tmp/2
   run $sptk4/aeq tmp/1 tmp/2
   [ "$status" -eq 0 ]

   # standard deviation
   $sptk3/vstat -l 2 tmp/0 -o 2 -d | $sptk3/sopr -SQRT > tmp/1
   $sptk4/vstat -l 2 tmp/0 -o 3 > tmp/2
   run $sptk4/aeq tmp/1 tmp/2
   [ "$status" -eq 0 ]

   # output interval
   $sptk3/vstat -l 2 tmp/0 -o 1 -t 5 > tmp/1
   $sptk4/vstat -l 2 tmp/0 -o 1 -t 5 > tmp/2
   run $sptk4/aeq tmp/1 tmp/2
   [ "$status" -eq 0 ]
}

@test "vstat: valgrind" {
   $sptk3/nrand -l 10 > tmp/1
   run valgrind $sptk4/vstat tmp/1
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
