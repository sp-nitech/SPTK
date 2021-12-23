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
tmp=test_mglsp2sp

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "mglsp2sp: compatibility" {
    $sptk3/ramp -s 0.01 -l 10 -t 0.04 > $tmp/1
    for i in $(seq 0 3); do
        $sptk3/mgclsp2sp -m 9 -l 8 -s 1 -a 0.1 -q "$i" -o "$i" -L $tmp/1 > $tmp/2
        $sptk4/mglsp2sp -m 9 -l 8 -s 1 -a 0.1 -q "$i" -o "$i" $tmp/1 > $tmp/3
        run $sptk4/aeq $tmp/2 $tmp/3
        [ "$status" -eq 0 ]
    done

    $sptk3/mgclsp2sp -m 10 -l 8 -s 1 -k $tmp/1 > $tmp/2
    $sptk4/mglsp2sp -m 10 -l 8 -s 1 -k 2 $tmp/1 > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "mglsp2sp: valgrind" {
    $sptk3/ramp -s 0.01 -l 10 -t 0.04 > $tmp/1
    run valgrind $sptk4/mglsp2sp -m 9 -l 8 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
