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
tmp=test_dfs

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "dfs: compatibility" {
    # -a
    $sptk3/nrand -l 20 | $sptk3/dfs -a 2 0.1 > $tmp/1
    $sptk3/nrand -l 20 | $sptk4/dfs -a 2 0.1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # -b
    $sptk3/nrand -l 20 | $sptk3/dfs -b 2 0.1 > $tmp/1
    $sptk3/nrand -l 20 | $sptk4/dfs -b 2 0.1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # -p and -z
    $sptk3/nrand -l 5 -s 123 -v 0.1 > $tmp/1
    $sptk3/nrand -l 5 -s 234 -v 0.1 > $tmp/2
    $sptk3/nrand -l 20 | $sptk3/dfs -p $tmp/1 -z $tmp/2 > $tmp/3
    $sptk3/nrand -l 20 | $sptk4/dfs -p $tmp/1 -z $tmp/2 > $tmp/4
    run $sptk4/aeq $tmp/3 $tmp/4
    [ "$status" -eq 0 ]
}

@test "dfs: identity" {
    $sptk3/nrand -l 20 > $tmp/1
    $sptk4/dfs $tmp/1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "dfs: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/dfs -a 4 3 -b 2 1 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
