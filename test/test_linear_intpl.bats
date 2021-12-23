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
tmp=test_linear_intpl

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "linear_intpl: compatibility" {
    echo 0 2 2 2 3 0 5 1 | $sptk3/x2x +ad > $tmp/0
    $sptk3/linear_intpl -m 10 -i 0.5 -j 5.0 $tmp/0 > $tmp/1
    $sptk4/linear_intpl -m 10 -s 0.5 -e 5.0 $tmp/0 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "linear_intpl: identity" {
    echo 0 10 1 11 2 12 3 13 4 14 | $sptk3/x2x +ad > $tmp/0
    echo 10 11 12 13 14 | $sptk3/x2x +ad > $tmp/1
    $sptk4/linear_intpl -l 5 $tmp/0 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "linear_intpl: valgrind" {
    echo 0 2 2 2 3 0 5 1 | $sptk3/x2x +ad > $tmp/0
    run valgrind $sptk4/linear_intpl -m 4 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
