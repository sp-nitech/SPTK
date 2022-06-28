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
tmp=test_delay

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "delay: compatibility" {
    for s in 3 13; do
        $sptk3/nrand -l 10 | $sptk3/delay -s $s > $tmp/1
        $sptk3/nrand -l 10 | $sptk4/delay -s $s > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]

        $sptk3/nrand -l 10 | $sptk3/delay -s $s -f > $tmp/3
        $sptk3/nrand -l 10 | $sptk4/delay -s $s -k > $tmp/4
        run $sptk4/aeq $tmp/3 $tmp/4
        [ "$status" -eq 0 ]
    done
}

@test "delay: identity" {
    $sptk3/nrand -l 20 > $tmp/1
    $sptk4/delay -s 0 $tmp/1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "delay: reversibility" {
    $sptk3/nrand -l 20 > $tmp/1
    $sptk4/delay -s 3 $tmp/1 | $sptk4/delay -s -3 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "delay: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/delay -s 3 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
