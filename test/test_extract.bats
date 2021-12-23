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
tmp=test_extract

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "extract: compatibility" {
    $sptk3/nrand -s 123 -l 256 | $sptk3/lbg -l 4 -e 8 -i 5 $tmp/1 > /dev/null
    $sptk3/nrand -s 234 -l 128 | $sptk3/extract -l 4 -i 0 $tmp/1 > $tmp/2
    $sptk3/nrand -s 234 -l 128 | $sptk4/extract -l 4 -i 0 $tmp/1 > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "extract: valgrind" {
    $sptk3/step -l 4 | $sptk3/x2x +di > $tmp/1
    $sptk3/nrand -l 4 > $tmp/2
    run valgrind $sptk4/extract -l 1 -i 0 $tmp/2 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
