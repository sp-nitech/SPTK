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
tmp=test_delta

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "delta: compatibility" {
    # -d odd
    $sptk3/nrand -l 20 | $sptk3/delta -l 2 -d -0.5 0 0.5 -d 1 -2 1 > $tmp/1
    $sptk3/nrand -l 20 | $sptk4/delta -l 2 -d -0.5 0 0.5 -d 1 -2 1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # -d even
    $sptk3/nrand -l 20 | $sptk3/delta -l 2 -d -1 -0.5 0 0.5 -d 2 -2 > $tmp/1
    $sptk3/nrand -l 20 | $sptk4/delta -l 2 -d -1 -0.5 0 0.5 -d 2 -2 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # -r
    $sptk3/nrand -l 20 | $sptk3/delta -l 2 -r 2 2 3 > $tmp/1
    $sptk3/nrand -l 20 | $sptk4/delta -l 2 -r 2 3 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # -R with -M option is removed from SPTK4.
}

@test "delta: identity" {
    $sptk3/nrand -l 20 > $tmp/1
    $sptk4/delta -l 10 $tmp/1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "delta: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/delta -l 10 -d -0.5 0 0.5 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
