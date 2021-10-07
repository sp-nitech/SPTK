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
tmp=test_rmse

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "rmse: compatibility" {
    $sptk3/nrand -s 123 -l 50 > $tmp/1
    $sptk3/nrand -s 321 -l 50 > $tmp/2
    $sptk3/rmse $tmp/1 $tmp/2 > $tmp/3
    $sptk4/rmse $tmp/1 $tmp/2 > $tmp/4
    run $sptk4/aeq $tmp/3 $tmp/4
    [ "$status" -eq 0 ]

    $sptk3/rmse $tmp/1 $tmp/2 -l 10 > $tmp/5
    $sptk4/rmse $tmp/1 $tmp/2 -l 10 -f > $tmp/6
    run $sptk4/aeq $tmp/5 $tmp/6
    [ "$status" -eq 0 ]
}

@test "rmse: magic number" {
    $sptk3/nrand -s 123 -l 50 > $tmp/1
    $sptk3/nrand -s 321 -l 50 > $tmp/2
    $sptk4/rmse $tmp/1 $tmp/2 > $tmp/3

    magic=100
    $sptk3/step -v $magic -l 10 > $tmp/4
    cat $tmp/1 $tmp/4 > $tmp/5
    cat $tmp/2 $tmp/4 > $tmp/6

    $sptk4/rmse -magic $magic $tmp/5 $tmp/6 > $tmp/7
    run $sptk4/aeq $tmp/3 $tmp/7
    [ "$status" -eq 0 ]
}

@test "rmse: valgrind" {
    $sptk3/nrand -s 123 -l 10 > $tmp/1
    $sptk3/nrand -s 321 -l 10 > $tmp/2
    run valgrind $sptk4/rmse $tmp/1 $tmp/2 -l 2 -f
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
