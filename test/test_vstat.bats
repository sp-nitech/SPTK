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
tmp=test_vstat

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "vstat: compatibility" {
    $sptk3/nrand -l 100 > $tmp/0

    ary1=("-o 0" "-o 1" "-o 2" "-o 2 -d" "-o 2 -r" "-o 2 -i")
    ary2=("-o 0" "-o 1" "-o 2" "-o 2 -d" "-o 4"    "-o 5")
    for i in $(seq 0 $((${#ary1[@]} - 1))); do
        # shellcheck disable=SC2086
        $sptk3/vstat -l 2 $tmp/0 ${ary1[$i]} > $tmp/1
        # shellcheck disable=SC2086
        $sptk4/vstat -l 2 $tmp/0 ${ary2[$i]} > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done

    # Lower and upper bounds:
    $sptk3/vstat -l 2 $tmp/0 -o 3 -c 90 > $tmp/1
    $sptk4/vstat -l 2 $tmp/0 -o 6 -c 90 > $tmp/2
    $sptk3/bcut +d $tmp/2 -s 0 -e 1 > $tmp/2_0
    $sptk3/bcut +d $tmp/2 -s 2 -e 3 > $tmp/2_2
    $sptk3/bcut +d $tmp/2 -s 4 -e 5 > $tmp/2_1
    cat $tmp/2_0 $tmp/2_1 $tmp/2_2 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # Standard deviation:
    $sptk3/vstat -l 2 $tmp/0 -o 2 -d | $sptk3/sopr -SQRT > $tmp/1
    $sptk4/vstat -l 2 $tmp/0 -o 3 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # Output interval:
    $sptk3/vstat -l 2 $tmp/0 -o 1 -t 5 > $tmp/1
    $sptk4/vstat -l 2 $tmp/0 -o 1 -t 5 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "vstat: valgrind" {
    $sptk3/nrand -l 10 > $tmp/1
    run valgrind $sptk4/vstat $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
