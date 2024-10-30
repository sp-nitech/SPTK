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
tmp=test_mode

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "mode: compatibility" {
    echo 0 1 0 1 1 2 | $sptk3/x2x +ad | $sptk4/mode -b 3 -c $tmp/3 > $tmp/1
    echo 1 0 2 | $sptk3/x2x +ad > $tmp/2
    echo 3 2 1 | $sptk3/x2x +ai > $tmp/4
    run diff $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
    run diff $tmp/3 $tmp/4
    [ "$status" -eq 0 ]

    echo 0 3 3 4 8 8 8 1 | $sptk3/x2x +ad | $sptk4/mode -l 4 -w 0 > $tmp/1
    echo 3 8 | $sptk3/x2x +ad > $tmp/2
    run diff $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "mode: valgrind" {
    $sptk3/nrand -l 20 | $sptk3/sopr -FIX > $tmp/1
    run valgrind $sptk4/mode -l 1 -o 0 -w 0 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
    run valgrind $sptk4/mode -l 1 -o 0 -w 1 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
