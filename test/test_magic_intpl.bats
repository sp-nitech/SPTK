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
tmp=test_magic_intpl

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "magic_intpl: equality" {
    echo 0 0 1 2 3 0 0 6 7 0 9 0 | $sptk3/x2x +ad > $tmp/1
    echo 1 1 1 2 3 4 5 6 7 8 9 9 | $sptk3/x2x +ad > $tmp/2
    $sptk4/magic_intpl $tmp/1 -magic 0 > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "magic_intpl: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/magic_intpl $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
