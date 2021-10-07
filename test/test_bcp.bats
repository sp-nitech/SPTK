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
tmp=test_bcp

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "bcp: compatibility" {
    ary1=("c" "C" "s" "S" "i3" "I3" "i" "I" "l" "L" "f" "d" "de")
    ary2=("c" "C" "s" "S" "h"  "H"  "i" "I" "l" "L" "f" "d" "e")
    for t in $(seq 0 $((${#ary1[@]}-1))); do
        $sptk3/ramp -l 20 | $sptk3/x2x +d"${ary1[$t]}" > $tmp/1
        $sptk3/bcp +"${ary1[$t]}" $tmp/1 -s 2 -e 3 -l 5 -L 5 -S 2 -f -1 |
            $sptk3/x2x +"${ary1[$t]}"d > $tmp/2
        $sptk4/bcp +"${ary2[$t]}" $tmp/1 -s 2 -e 3 -l 5 -L 5 -S 2 -f -1 |
            $sptk3/x2x +"${ary1[$t]}"d > $tmp/3
        run $sptk4/aeq $tmp/2 $tmp/3
        [ "$status" -eq 0 ]
    done

    # Note that SPTK3 does not support +a with -f option.
    $sptk3/ramp -l 20 | $sptk3/x2x +da > $tmp/1
    $sptk3/bcp +a $tmp/1 -s 2 -e 3 -l 5 | $sptk3/x2x +ad > $tmp/2
    $sptk4/bcp +a $tmp/1 -s 2 -e 3 -l 5 | $sptk3/x2x +ad > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "bcp: identity" {
    $sptk3/nrand -l 20 > $tmp/1
    $sptk4/bcp +d -l 1 $tmp/1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "bcp: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/bcp +d -s 1 -e 5 -l 10 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
