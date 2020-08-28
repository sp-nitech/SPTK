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
data=asset/data.short

setup() {
   mkdir -p tmp
}

teardown() {
   rm -rf tmp
}

# Note `zerodf -i 0` has no compatibility due to the change of implementation.
@test "zerodf: compatibility (standard form)" {
   $sptk3/x2x +sd $data | $sptk3/frame -l 400 -p 80 | \
      $sptk3/window -l 400 -w 1 -n 1 | \
      $sptk3/lpc -l 400 -m 24 > tmp/1
   # Calculate impulse response of all-pole filter.
   $sptk3/train -l `expr 19200 / 80 \* 240` -p 240 -n 0 | \
      $sptk3/poledf -m 24 -p 240 -i 0 tmp/1 > tmp/2
   $sptk3/nrand -l 19200 | $sptk3/zerodf -m 239 -p 80 tmp/2 > tmp/3
   $sptk3/nrand -l 19200 | $sptk4/zerodf -m 239 -p 80 tmp/2 > tmp/4
   run $sptk4/aeq -L tmp/3 tmp/4
   [ "$status" -eq 0 ]
}

@test "zerodf: compatibility (transposed form)" {
   $sptk3/x2x +sd $data | $sptk3/frame -l 400 -p 80 | \
      $sptk3/window -l 400 -w 1 -n 1 | \
      $sptk3/lpc -l 400 -m 24 > tmp/1
   $sptk3/train -l `expr 19200 / 80 \* 240` -p 240 -n 0 | \
      $sptk3/poledf -m 24 -p 240 -i 0 tmp/1 > tmp/2
   $sptk3/nrand -l 19200 | $sptk3/zerodf -m 239 -p 80 -t tmp/2 > tmp/3
   $sptk3/nrand -l 19200 | $sptk4/zerodf -m 239 -p 80 -t tmp/2 > tmp/4
   run $sptk4/aeq -L tmp/3 tmp/4
   [ "$status" -eq 0 ]
}

@test "zerodf: compatibility (without gain)" {
   $sptk3/x2x +sd $data | $sptk3/frame -l 400 -p 80 | \
      $sptk3/window -l 400 -w 1 -n 1 | \
      $sptk3/lpc -l 400 -m 24 > tmp/1
   $sptk3/train -l `expr 19200 / 80 \* 240` -p 240 -n 0 | \
      $sptk3/poledf -m 24 -p 240 -i 0 tmp/1 > tmp/2
   $sptk3/nrand -l 19200 | $sptk3/zerodf -m 239 -p 80 -k tmp/2 > tmp/3
   $sptk3/nrand -l 19200 | $sptk4/zerodf -m 239 -p 80 -k tmp/2 > tmp/4
   run $sptk4/aeq -L tmp/3 tmp/4
   [ "$status" -eq 0 ]
}

@test "zerodf: identity" {
   $sptk3/step -l 10 > tmp/1
   $sptk3/nrand -l 10 > tmp/2
   $sptk4/zerodf -m 0 -i 0 -p 1 tmp/1 tmp/2 > tmp/3
   run $sptk4/aeq tmp/2 tmp/3
   [ "$status" -eq 0 ]
}

@test "zerodf: valgrind" {
   $sptk3/nrand -l 10 > tmp/1
   $sptk3/nrand -l 10 > tmp/2
   run valgrind $sptk4/zerodf -m 1 -i 0 -p 1 tmp/1 tmp/2 > /dev/null
   [ $(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/') -eq 0 ]
}
