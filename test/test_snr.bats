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
tmp=test_snr

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "snr: compatibility" {
    $sptk3/nrand -s 123 -l 50 > $tmp/1
    $sptk3/nrand -s 321 -l 50 > $tmp/2

    # SNR
    $sptk3/snr -o 2 $tmp/1 $tmp/2 > $tmp/3
    $sptk4/snr -o 0 $tmp/1 $tmp/2 > $tmp/4
    run $sptk4/aeq $tmp/3 $tmp/4
    [ "$status" -eq 0 ]

    # segmental SNR
    $sptk3/snr -o 3 -l 10 $tmp/1 $tmp/2 > $tmp/5
    $sptk4/snr -o 1 -l 10 $tmp/1 $tmp/2 > $tmp/6
    run $sptk4/aeq $tmp/5 $tmp/6
    [ "$status" -eq 0 ]

    # segmental SNR per frame
    $sptk3/snr -o 1 -l 10 $tmp/1 $tmp/2 |
        grep \~ | tr -s ' ' | cut -d' ' -f 6 | $sptk3/x2x +ad > $tmp/7
    $sptk4/snr -o 2 -l 10 $tmp/1 $tmp/2 > $tmp/8
    run $sptk4/aeq $tmp/7 $tmp/8
    [ "$status" -eq 0 ]
}

@test "snr: valgrind" {
    $sptk3/nrand -s 123 -l 10 > $tmp/1
    $sptk3/nrand -s 321 -l 10 > $tmp/2
    run valgrind $sptk4/snr $tmp/1 $tmp/2
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
