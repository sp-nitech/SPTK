#!/usr/bin/env bats
# ------------------------------------------------------------------------ #
# Copyright 2021 SPTK Working Group                                        #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ------------------------------------------------------------------------ #

sptk3=tools/sptk/bin
sptk4=bin
tmp=test_x2x

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "x2x: compatibility" {
    min=(-128   0 -32768     0 -8388608        0 -2147483648          0
         -9223372036854775808                    0 -1e+37 -1e+308 -1e+308)
    max=( 127 255  32767 65535  8388607 16777215  2147483647 4294967295
          9223372036854775807 18446744073709551615  1e+37  1e+308  1e+308)
    ary1=("c" "C" "s" "S" "i3" "I3" "i" "I" "l" "L" "f" "d" "de")
    ary2=("c" "C" "s" "S"  "h"  "H" "i" "I" "l" "L" "f" "d"  "e")
    for t in $(seq 0 $((${#ary1[@]}-1))); do
        echo "${min[$t]}" "${max[$t]}" | $sptk3/x2x +a"${ary1[$t]}" |
            $sptk3/x2x +"${ary1[$t]}"d > $tmp/1
        echo "${min[$t]}" "${max[$t]}" | $sptk4/x2x +a"${ary2[$t]}" |
            $sptk3/x2x +"${ary1[$t]}"d > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done
}

@test "x2x: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/x2x +da $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
