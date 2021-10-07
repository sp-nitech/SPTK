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
tmp=test_huffman_encode

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "huffman_encode: reversibility" {
    $sptk3/nrand -l 8 | $sptk4/huffman > $tmp/1
    $sptk3/ramp -s 7 -t -1 -l 8 > $tmp/2
    $sptk3/x2x +di $tmp/2 | $sptk4/huffman_encode $tmp/1 |
        $sptk4/huffman_decode $tmp/1 | $sptk3/x2x +id > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "huffman_encode: valgrind" {
    $sptk3/nrand -l 8 | $sptk4/huffman > $tmp/1
    $sptk3/ramp -l 8 | $sptk3/x2x +di > $tmp/2
    run valgrind $sptk4/huffman_encode $tmp/1 $tmp/2
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
