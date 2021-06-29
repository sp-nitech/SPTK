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

@test "x2x: compatibility" {
   min=(-128   0 -32768     0 -8388608        0 -2147483648          0
        -9223372036854775808                    0 -1e+37 -1e+308 -1e+308)
   max=( 127 255  32767 65535  8388607 16777215  2147483647 4294967295
         9223372036854775807 18446744073709551615  1e+37  1e+308  1e+308)
   ary1=("c" "C" "s" "S" "i3" "I3" "i" "I" "l" "L" "f" "d" "de")
   ary2=("c" "C" "s" "S"  "h"  "H" "i" "I" "l" "L" "f" "d"  "e")
   for t in $(seq 0 $((${#ary1[@]}-1))); do
      echo ${min[$t]} ${max[$t]} | $sptk3/x2x +a${ary1[$t]} | \
         $sptk3/x2x +${ary1[$t]}d > tmp/1
      echo ${min[$t]} ${max[$t]} | $sptk4/x2x +a${ary2[$t]} | \
         $sptk3/x2x +${ary1[$t]}d > tmp/2
      run $sptk4/aeq tmp/1 tmp/2
      [ "$status" -eq 0 ]
   done
}

@test "x2x: valgrind" {
   $sptk3/nrand -l 20 > tmp/1
   run valgrind $sptk4/x2x +da tmp/1
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
