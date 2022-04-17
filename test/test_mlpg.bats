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
tmp=test_mlpg

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "mlpg: compatibility" {
    # Make mean.
    $sptk3/nrand -s 1 -l 200 > $tmp/1
    # Make variance.
    $sptk3/nrand -s 2 -l 200 | $sptk3/sopr -ABS -m 0.01 > $tmp/2
    # Make pdf.
    $sptk3/merge +d -l 10 -L 10 $tmp/1 $tmp/2 > $tmp/3

    # Perform MLPG.
    $sptk3/mlpg -l 5 -r 2 2 3 $tmp/3 | $sptk3/x2x +da | grep -v ^0$ |
        $sptk3/x2x +ad > $tmp/4
    $sptk4/mlpg -l 5 -r 2 3 $tmp/3 > $tmp/5
    run $sptk4/aeq -L $tmp/4 $tmp/5
    [ "$status" -eq 0 ]

    # Remove zeros because SPTK3 does not consider delay.
    $sptk3/mlpg -l 5 -d -0.5 0 0.5 $tmp/3 | $sptk3/x2x +da | grep -v ^0$ |
        $sptk3/x2x +ad > $tmp/6
    $sptk4/mlpg -l 5 -d -0.5 0 0.5 $tmp/3 > $tmp/7
    run $sptk4/aeq -t 1e-5 -L -e 1 $tmp/6 $tmp/7
    [ "$status" -eq 0 ]

    # Check consistency between recursive and non-recursive mode.
    $sptk4/mlpg -l 5 -d -0.5 0 0.5 -R 1 $tmp/3 > $tmp/8
    run $sptk4/aeq $tmp/7 $tmp/8
    [ "$status" -eq 0 ]
}

@test "mlpg: valgrind" {
    $sptk3/nrand -l 20 | $sptk3/sopr -ABS > $tmp/1
    run valgrind $sptk4/mlpg -l 2 -R 0 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
    run valgrind $sptk4/mlpg -l 2 -R 1 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
