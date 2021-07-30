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

@test "lsp2lpc: compatibility" {
   $sptk3/nrand -l 800 | $sptk3/lpc -l 400 -m 12 | \
      $sptk3/lpc2lsp -m 12 > tmp/1
   ary=("" "-L" "-k")
   for k in $(seq 0 2); do
      $sptk3/lsp2lpc -m 12 ${ary[$k]} tmp/1 > tmp/2
      $sptk4/lsp2lpc -m 12 -k $k tmp/1 > tmp/3
      run $sptk4/aeq tmp/2 tmp/3
      [ "$status" -eq 0 ]
   done

   for q in $(seq 0 3); do
      $sptk3/lsp2lpc -m 12 -q $q tmp/1 > tmp/2
      $sptk4/lsp2lpc -m 12 -q $q tmp/1 > tmp/3
      run $sptk4/aeq tmp/2 tmp/3
      [ "$status" -eq 0 ]
   done

   $sptk3/nrand -l 800 | $sptk3/lpc -l 400 -m 11 | \
      $sptk3/lpc2lsp -m 11 > tmp/1
   $sptk3/lsp2lpc -m 11 tmp/1 > tmp/2
   $sptk4/lsp2lpc -m 11 tmp/1 > tmp/3
}

@test "lsp2lpc: reversibility" {
   $sptk3/nrand -l 400 | $sptk3/lpc -l 400 -m 12 | \
      $sptk3/lpc2lsp -m 12 -n 256 > tmp/1
   $sptk4/lsp2lpc -m 12 tmp/1 | $sptk4/lpc2lsp -m 12 > tmp/2
   run $sptk4/aeq tmp/1 tmp/2
   [ "$status" -eq 0 ]
}

@test "lsp2lpc: valgrind" {
   $sptk3/nrand -l 800 | $sptk3/lpc -l 400 -m 12 | \
      $sptk3/lpc2lsp -m 12 -n 256 > tmp/1
   run valgrind $sptk4/lsp2lpc -m 12 tmp/1
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
