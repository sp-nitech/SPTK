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
tmp=test_spec

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "spec: compatibility" {
    $sptk3/nrand -l 32 > $tmp/0

    for o in $(seq 0 3); do
        $sptk3/spec -l 64 -o "$o" $tmp/0 > $tmp/1
        $sptk4/spec -l 64 -o "$o" $tmp/0 > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done

    # SPTK4 supports -o 2 or -o 3 with -e.
    for o in $(seq 0 1); do
        $sptk3/spec -l 64 -o "$o" -e 1e-5 $tmp/0 > $tmp/1
        $sptk4/spec -l 64 -o "$o" -e 1e-5 $tmp/0 > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done

    # SPTK3 does not support -o 2 or -o 3 with -E.
    for o in $(seq 0 1); do
        $sptk3/spec -l 64 -o "$o" -E -40 $tmp/0 > $tmp/1
        $sptk4/spec -l 64 -o "$o" -E -40 $tmp/0 > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done

    # -z and -p
    echo +0.42 1 | $sptk3/x2x +ad > $tmp/3
    echo 1 -0.42 | $sptk3/x2x +ad > $tmp/4
    $sptk3/spec -l 16 -m 1 -n 1 -z $tmp/3 -p $tmp/4 -o 0 > $tmp/5
    $sptk4/spec -l 16 -m 1 -n 1 -z $tmp/3 -p $tmp/4 -o 0 > $tmp/6
    run $sptk4/aeq $tmp/5 $tmp/6
    [ "$status" -eq 0 ]
}

@test "spec: valgrind" {
    $sptk3/nrand -l 128 > $tmp/1
    run valgrind $sptk4/spec -l 16 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
