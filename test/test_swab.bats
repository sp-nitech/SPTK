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

@test "swab: compatibility" {
   ary1=("s" "S" "i3" "I3" "i" "I" "l" "L" "f" "d" "de")
   ary2=("s" "S" "h"  "H"  "i" "I" "l" "L" "f" "d" "e")
   for t in $(seq 0 $((${#ary1[@]}-1))); do
      $sptk3/ramp -l 20 | $sptk3/x2x +d${ary1[$t]} > tmp/1

      $sptk3/swab +${ary1[$t]} tmp/1 -s 2 -e 10 | \
         $sptk3/x2x +${ary1[$t]}d > tmp/2
      $sptk4/swab +${ary2[$t]} tmp/1 -s 2 -e 10 | \
         $sptk3/x2x +${ary1[$t]}d > tmp/3
      run $sptk4/aeq tmp/2 tmp/3
      [ "$status" -eq 0 ]

      $sptk3/swab +${ary1[$t]} tmp/1 -S 2 -E 10 | \
         $sptk3/x2x +${ary1[$t]}d > tmp/4
      $sptk4/swab +${ary2[$t]} tmp/1 -S 2 -E 10 | \
         $sptk3/x2x +${ary1[$t]}d > tmp/5
      run $sptk4/aeq tmp/4 tmp/5
      [ "$status" -eq 0 ]
   done
}

@test "swab: identity" {
   $sptk3/nrand -l 20 > tmp/1
   $sptk4/swab +d tmp/1 | $sptk4/swab +d > tmp/2
   run $sptk4/aeq tmp/1 tmp/2
   [ "$status" -eq 0 ]
}

@test "swab: valgrind" {
   $sptk3/nrand -l 20 > tmp/1
   run valgrind $sptk4/swab +d tmp/1
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
