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
tmp=test_dtw_merge

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "dtw_merge: compatibility" {
    $sptk3/nrand -s 1 -l 100 > $tmp/0_q
    $sptk3/nrand -s 2 -l 80 > $tmp/0_r

    $sptk3/dtw -l 2 $tmp/0_r $tmp/0_q -v $tmp/1 > /dev/null
    $sptk3/dtw -l 2 $tmp/0_r $tmp/0_q -V $tmp/1 > $tmp/2
    $sptk4/dtw -l 2 $tmp/0_r $tmp/0_q -P $tmp/3 > /dev/null
    $sptk4/dtw_merge -l 2 $tmp/3 $tmp/0_r $tmp/0_q > $tmp/4
    run $sptk4/aeq $tmp/2 $tmp/4
    [ "$status" -eq 0 ]
}

@test "dtw_merge: valgrind" {
    echo 0 0 1 1 | $sptk3/x2x +ai > $tmp/1
    $sptk3/nrand -l 2 > $tmp/2
    run valgrind $sptk4/dtw_merge -l 1 $tmp/1 $tmp/2 $tmp/2
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
