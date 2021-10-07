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
tmp=test_mfcc

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "mfcc: compatibility" {
    # Note that there is no compatibility when -w 0 -E: SPTK3 calculates the
    # signal energy before windowing whereas SPTK4 calculates one after
    # windowing.
    $sptk3/nrand -l 16 |
        $sptk3/mfcc -a 0 -c 10 -e 1 -l 8 -L 8 -w 1 -n 6 -m 3 -E -0 > $tmp/1
    $sptk3/nrand -l 16 |
        $sptk4/mfcc -c 10 -e 1 -l 8 -n 6 -m 3 -o 3 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "mfcc: valgrind" {
    $sptk3/nrand -l 16 > $tmp/1
    run valgrind $sptk4/mfcc -l 8 -n 4 -m 3 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
