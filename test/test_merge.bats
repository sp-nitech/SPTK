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
tmp=test_merge

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "merge: compatibility" {
    ary1=("c" "C" "s" "S" "i3" "I3" "i" "I" "l" "L" "f" "d" "de")
    ary2=("c" "C" "s" "S" "h" "H" "i" "I" "l" "L" "f" "d" "e")
    for t in $(seq 0 $((${#ary1[@]} - 1))); do
        $sptk3/ramp -l 20 | $sptk3/x2x +d"${ary1[$t]}" > $tmp/1
        $sptk3/ramp -s 20 -l 20 | $sptk3/x2x +d"${ary1[$t]}" > $tmp/2

        $sptk3/merge +"${ary1[$t]}" $tmp/1 $tmp/2 -l 4 -L 5 |
            $sptk3/x2x +"${ary1[$t]}"d > $tmp/3
        $sptk4/merge +"${ary2[$t]}" $tmp/1 $tmp/2 -l 4 -L 5 |
            $sptk3/x2x +"${ary1[$t]}"d > $tmp/4
        run $sptk4/aeq $tmp/3 $tmp/4
        [ "$status" -eq 0 ]

        $sptk3/merge +"${ary1[$t]}" $tmp/1 $tmp/2 -s 2 -l 6 -L 4 |
            $sptk3/x2x +"${ary1[$t]}"d > $tmp/3
        $sptk4/merge +"${ary2[$t]}" $tmp/1 $tmp/2 -s 2 -l 6 -L 4 |
            $sptk3/x2x +"${ary1[$t]}"d > $tmp/4
        run $sptk4/aeq -L $tmp/3 $tmp/4
        [ "$status" -eq 0 ]

        $sptk3/merge +"${ary1[$t]}" $tmp/1 $tmp/2 -s 2 -l 6 -L 4 -o |
            $sptk3/x2x +"${ary1[$t]}"d > $tmp/3
        $sptk4/merge +"${ary2[$t]}" $tmp/1 $tmp/2 -s 2 -l 6 -L 4 -w |
            $sptk3/x2x +"${ary1[$t]}"d > $tmp/4
        run $sptk4/aeq -L $tmp/3 $tmp/4
        [ "$status" -eq 0 ]
    done
}

@test "merge: recursive" {
    echo 4 | $sptk3/x2x +ad > $tmp/1
    echo 1 1 2 2 3 3 | $sptk3/x2x +ad > $tmp/2
    $sptk4/merge +d $tmp/1 $tmp/2 -q 1 -s 0 -l 2 -L 1 > $tmp/3
    echo 4 1 1 4 2 2 4 3 3 | $sptk3/x2x +ad > $tmp/4
    run $sptk4/aeq $tmp/3 $tmp/4
    [ "$status" -eq 0 ]
}

@test "merge: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    for q in $(seq 0 1); do
        run valgrind $sptk4/merge +d -q "$q" $tmp/1 $tmp/1 -l 1 -L 1
        [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
    done
}
