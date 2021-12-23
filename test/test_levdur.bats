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
tmp=test_levdur
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "levdur: compatibility" {
    $sptk3/x2x +sd $data | $sptk3/frame -l 400 -p 80 |
        $sptk3/window -l 400 -w 1 -n 1 |
        $sptk3/acorr -l 400 -m 20 > $tmp/1
    $sptk3/levdur -m 20 $tmp/1 > $tmp/2
    $sptk4/levdur -m 20 $tmp/1 > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "levdur: reversibility" {
    $sptk3/x2x +sd $data | $sptk3/frame -l 400 -p 80 |
        $sptk3/window -l 400 -w 1 -n 1 |
        $sptk3/acorr -l 400 -m 20 > $tmp/1
    $sptk4/levdur -m 20 $tmp/1 |
        $sptk4/rlevdur -m 20 > $tmp/2
    run $sptk4/aeq -e 1 $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "levdur: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/levdur -m 9 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
