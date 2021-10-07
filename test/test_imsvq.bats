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
tmp=test_imsvq

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "imsvq: compatibility" {
    $sptk3/nrand -s 123 -l 32 > $tmp/1
    $sptk3/nrand -s 234 -l 32 > $tmp/2
    $sptk3/nrand -s 345 -l 32 > $tmp/3
    echo 3 7 4 0 | $sptk3/x2x +ai |
        $sptk3/imsvq -s 8 $tmp/1 -s 8 $tmp/2 -s 8 $tmp/3 -l 4 > $tmp/4
    echo 3 7 4 0 | $sptk3/x2x +ai |
        $sptk4/imsvq -s $tmp/1 -s $tmp/2 -s $tmp/3 -l 4 > $tmp/5
    run $sptk4/aeq $tmp/4 $tmp/5
    [ "$status" -eq 0 ]
}

@test "imsvq: valgrind" {
    $sptk3/nrand -l 32 > $tmp/1
    $sptk3/step -l 4 | $sptk3/x2x +di > $tmp/2
    run valgrind $sptk4/imsvq -s $tmp/1 -l 4 $tmp/2
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
