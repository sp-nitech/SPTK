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
tmp=test_pnorm

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "pnorm: compatibility" {
    $sptk3/nrand -l 20 |
        $sptk3/mgc2mgc -m 9 -M 127 -a 0.1 -G 1 -U |
        $sptk3/sopr -P |
        $sptk3/vsum -t 128 |
        $sptk3/sopr -LN -m 0.5 > $tmp/1
    $sptk3/nrand -l 20 |
        $sptk4/pnorm -m 9 -l 128 -a 0.1 |
        $sptk3/bcp +d -n 10 -e 0 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "pnorm: reversibility" {
    $sptk3/nrand -l 20 > $tmp/1
    $sptk4/pnorm -m 9 $tmp/1 | $sptk4/ipnorm -m 9 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "pnorm: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/pnorm -m 9 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
