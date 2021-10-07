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
tmp=test_ramp

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "ramp: compatibility" {
    # -l
    $sptk3/ramp -l 16 -s 1 -t 1 > $tmp/1
    $sptk4/ramp -l 16 -s 1 -t 1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # -n
    $sptk3/ramp -n 16 -s 1 -t -1 > $tmp/1
    $sptk4/ramp -m 16 -s 1 -t -1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # -t (e > s)
    $sptk3/ramp -e 4 -s 0 -t 0.3 > $tmp/1
    $sptk4/ramp -e 4 -s 0 -t 0.3 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # -t (s > e)
    $sptk3/ramp -e 0 -s 4 -t -0.3 > $tmp/1
    $sptk4/ramp -e 0 -s 4 -t -0.3 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "ramp: valgrind" {
    run valgrind $sptk4/ramp -l 10
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
