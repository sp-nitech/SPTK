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
tmp=test_mgcep

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "mgcep: compatibility (g = 0)" {
    $sptk3/nrand -s 2 -l 32 | $sptk3/mcep -l 16 -m 4 -j 10 > $tmp/1
    $sptk3/nrand -s 2 -l 32 | $sptk4/mgcep -l 16 -m 4 -i 10 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "mgcep: compatibility (g < 0)" {
    for o in $(seq 0 3); do
        $sptk3/nrand -s 2 -l 32 | $sptk3/mgcep -l 16 -m 4 -g -0.5 -j 10 -o "$o" > $tmp/1
        $sptk3/nrand -s 2 -l 32 | $sptk4/mgcep -l 16 -m 4 -g -0.5 -i 10 -o "$o" > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done
}

@test "mgcep: valgrind" {
    $sptk3/nrand -l 32 > $tmp/1
    run valgrind $sptk4/mgcep -l 16 -m 4 -i 3 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
    run valgrind $sptk4/mgcep -l 16 -m 4 -g -1 -i 3 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
