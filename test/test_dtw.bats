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
tmp=test_dtw

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "dtw: compatibility" {
    $sptk3/nrand -s 1 -l 100 > $tmp/0_q
    $sptk3/nrand -s 2 -l 80 > $tmp/0_r

    for d in $(seq 0 1); do
        for p in $(seq 0 6); do
            $sptk3/dtw -l 2 -p $((p+1)) -n $((d+1)) \
                       $tmp/0_r $tmp/0_q -s $tmp/1_s > $tmp/1
            $sptk4/dtw -l 2 -p "$p" -d "$d" \
                       $tmp/0_r $tmp/0_q -S $tmp/2_s > $tmp/2
            run $sptk4/aeq $tmp/1 $tmp/2
            [ "$status" -eq 0 ]
            run $sptk4/aeq $tmp/1_s $tmp/2_s
            [ "$status" -eq 0 ]
        done
    done
}

@test "dtw: valgrind" {
    $sptk3/nrand -l 20 > $tmp/0
    run valgrind $sptk4/dtw -l 2 -p 4 $tmp/0 $tmp/0
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
