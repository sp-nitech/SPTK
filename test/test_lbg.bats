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

@test "lbg: compatibility" {
   $sptk3/nrand -s 123 -l 8 > tmp/1
   $sptk3/nrand -s 234 -l 512 > tmp/2

   # Note -n option in SPTK4 has no longer compatibility with -m option in SPTK3
   # due to the different implementation.
   $sptk3/lbg -l 4 -s 2 -e 32 -F tmp/1 -i 5 -m 1 -r 0.001 tmp/4 < tmp/2 > tmp/3
   $sptk4/lbg -l 4 -e 32 -C tmp/1 -i 5 -n 1 -I tmp/6 -r 0.001 < tmp/2 > tmp/5
   run $sptk4/aeq tmp/3 tmp/5
   [ "$status" -eq 0 ]

   $sptk4/x2x +id tmp/4 > tmp/7
   $sptk4/x2x +id tmp/6 > tmp/8
   run $sptk4/aeq tmp/7 tmp/8
   [ "$status" -eq 0 ]

   # Without initial codebook:
   $sptk3/lbg -l 4 -e 32 -i 5 -S 2 -r 0.0001 tmp/4 < tmp/2 > tmp/9
   $sptk4/lbg -l 4 -e 32 -i 5 -s 2 -r 0.0001 < tmp/2 > tmp/10
   run $sptk4/aeq tmp/9 tmp/10
   [ "$status" -eq 0 ]
}

@test "lbg: valgrind" {
   $sptk3/nrand -l 512 > tmp/1
   run valgrind $sptk4/lbg -l 4 -e 8 -i 10 tmp/1
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
