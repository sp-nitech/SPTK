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
tmp=test_window

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "window: compatibility" {
    $sptk3/nrand -l 20 > $tmp/1
    for w in $(seq 0 5); do
        $sptk3/window -w "$w" $tmp/1 > $tmp/2
        $sptk4/window -w "$w" $tmp/1 > $tmp/3
        run $sptk4/aeq $tmp/2 $tmp/3
        [ "$status" -eq 0 ]
    done
    for n in $(seq 0 2); do
        $sptk3/window -n "$n" $tmp/1 > $tmp/2
        $sptk4/window -n "$n" $tmp/1 > $tmp/3
        run $sptk4/aeq $tmp/2 $tmp/3
        [ "$status" -eq 0 ]
    done
}

@test "window: identity" {
    $sptk3/nrand -l 20 > $tmp/1
    $sptk4/window -w 5 -n 0 -l 10 $tmp/1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "window: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/window -l 10 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
