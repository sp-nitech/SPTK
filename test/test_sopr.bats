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

@test "sopr: compatibility" {
   $sptk3/nrand -l 10 | $sptk3/sopr -ABS > tmp/0

   ary1=(" " "-a 1" "-s 1" "-m 2" "-d 2" "-p 2" "-f 0" "-c 0"
         "-ABS" "-INV" "-P" "-R"
         "-LN" "-LOG2" "-LOG10" "-LOGX 20"
         "-EXP" "-POW2" "-POW10" "-POWX 20"
         "-FIX" "-UNIT" "-CLIP"
         "-SIN" "-COS" "-TAN" "-ATAN")
   ary2=(" " "-a 1" "-s 1" "-m 2" "-d 2" "-p 2" "-l 0" "-u 0"
         "-ABS" "-INV" "-SQR" "-SQRT"
         "-LN" "-LOG2" "-LOG10" "-LOGX 20"
         "-EXP" "-POW2" "-POW10" "-POWX 20"
         "-ROUND" "-UNIT" "-RAMP"
         "-SIN" "-COS" "-TAN" "-ATAN")
   for i in $(seq 0 $((${#ary1[@]} - 1))); do
      $sptk3/sopr tmp/0 ${ary1[$i]} > tmp/1
      $sptk4/sopr tmp/0 ${ary2[$i]} > tmp/2
      run $sptk4/aeq tmp/1 tmp/2
      [ "$status" -eq 0 ]
   done

   ary=("pi" "dB" "cent" "semitone" "octave" "sqrt100" "ln10" "exp1")
   for i in $(seq 0 $((${#ary[@]} - 1))); do
      $sptk3/sopr tmp/0 -m ${ary[$i]} > tmp/1
      $sptk4/sopr tmp/0 -m ${ary[$i]} > tmp/2
      run $sptk4/aeq tmp/1 tmp/2
      [ "$status" -eq 0 ]
   done

   ary=("-magic 0" "-magic 0 -MAGIC -1")
   for i in $(seq 0 $((${#ary[@]} - 1))); do
      $sptk3/ramp -l 3 | $sptk3/sopr ${ary[$i]} > tmp/1
      $sptk3/ramp -l 3 | $sptk4/sopr ${ary[$i]} > tmp/2
      run $sptk4/aeq tmp/1 tmp/2
      [ "$status" -eq 0 ]
   done
   $sptk3/ramp -l 3 > tmp/1
   $sptk4/sopr tmp/1 -magic 0 -MAGIC -1 -magic -1 -MAGIC 0 > tmp/2
   run $sptk4/aeq tmp/1 tmp/2
   [ "$status" -eq 0 ]
}

@test "sopr: valgrind" {
   $sptk3/nrand -l 20 > tmp/1
   run valgrind $sptk4/sopr -m 2 tmp/1
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
