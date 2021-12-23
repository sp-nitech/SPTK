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
tmp=test_amgcep

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "amgcep: compatibility (a > 0)" {
    $sptk3/nrand -l 20 | $sptk3/amcep -m 3 $tmp/3 > $tmp/1
    $sptk3/nrand -l 20 | $sptk4/amgcep -m 3 -E $tmp/4 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
    run $sptk4/aeq $tmp/3 $tmp/4
    [ "$status" -eq 0 ]
}

@test "amgcep: compatibility (c > 0)" {
    $sptk3/nrand -l 20 | $sptk3/agcep -m 3 -c 1 $tmp/3 > $tmp/1
    $sptk3/nrand -l 20 | $sptk4/amgcep -m 3 -c 1 -a 0 -E $tmp/4 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
    run $sptk4/aeq $tmp/3 $tmp/4
    [ "$status" -eq 0 ]
}

@test "amgcep: compatibility (a = 0, c = 0)" {
    $sptk3/nrand -l 20 | $sptk3/acep -m 3 > $tmp/1
    $sptk3/nrand -l 20 | $sptk4/amgcep -m 3 -a 0 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "amcep: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/amgcep -m 3 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
    run valgrind $sptk4/amgcep -m 3 -c 1 -a 0 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
