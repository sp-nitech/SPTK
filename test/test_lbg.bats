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
tmp=test_lbg

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "lbg: compatibility" {
    $sptk3/nrand -s 123 -l 8 > $tmp/1
    $sptk3/nrand -s 234 -l 512 > $tmp/2

    # Note -n option in SPTK4 has no longer compatibility with -m option in SPTK3
    # due to the different implementation.
    $sptk3/lbg -l 4 -s 2 -e 32 -F $tmp/1 -i 5 -m 1 -r 0.001 $tmp/4 < $tmp/2 > $tmp/3
    $sptk4/lbg -l 4 -e 32 -C $tmp/1 -i 5 -n 1 -I $tmp/6 -r 0.001 < $tmp/2 > $tmp/5
    run $sptk4/aeq $tmp/3 $tmp/5
    [ "$status" -eq 0 ]

    $sptk4/x2x +id $tmp/4 > $tmp/7
    $sptk4/x2x +id $tmp/6 > $tmp/8
    run $sptk4/aeq $tmp/7 $tmp/8
    [ "$status" -eq 0 ]

    # Without initial codebook:
    $sptk3/lbg -l 4 -e 32 -i 5 -S 2 -r 0.0001 $tmp/4 < $tmp/2 > $tmp/9
    $sptk4/lbg -l 4 -e 32 -i 5 -s 2 -r 0.0001 < $tmp/2 > $tmp/10
    run $sptk4/aeq $tmp/9 $tmp/10
    [ "$status" -eq 0 ]
}

@test "lbg: valgrind" {
    $sptk3/nrand -l 512 > $tmp/1
    run valgrind $sptk4/lbg -l 4 -e 8 -i 10 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
