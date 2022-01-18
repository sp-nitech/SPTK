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
tmp=test_mpir2c

setup() {
    mkdir -p $tmp
    $sptk3/nrand -l 32 | $sptk3/c2ir -m 15 -l 16 > $tmp/0
}

teardown() {
    rm -rf $tmp
}

@test "mpir2c: compatibility" {
    $sptk3/c2ir -i -l 16 -m 9 $tmp/0 > $tmp/1
    $sptk4/mpir2c -l 16 -M 9 $tmp/0 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    $sptk4/mgc2mgc -g 1 -G 0 -m 15 -M 9 -u $tmp/0 > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "mpir2c: reversibility" {
    $sptk4/mpir2c -m 15 -M 15 $tmp/0 | $sptk4/c2mpir -m 15 -M 15 > $tmp/1
    run $sptk4/aeq $tmp/0 $tmp/1
    [ "$status" -eq 0 ]
}

@test "mpir2c: valgrind" {
    run valgrind $sptk4/mpir2c -l 16 -M 20 $tmp/0
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
