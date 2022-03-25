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
tmp=test_acorr

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "acorr: compatibility" {
    # Time domain:
    $sptk3/nrand -l 20 | $sptk3/acorr -l 10 -m 4 > $tmp/1
    $sptk3/nrand -l 20 | $sptk4/acorr -l 10 -m 4 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    # Frequency domain:
    for o in $(seq 0 3); do
        $sptk3/nrand -l 20 | $sptk3/spec -m 9 -l 32 -o "$o" |
            $sptk4/acorr -l 32 -m 4 -q "$o" > $tmp/3
        run $sptk4/aeq $tmp/1 $tmp/3
        [ "$status" -eq 0 ]
    done
}

@test "acorr: normalization" {
    $sptk3/nrand -l 10 | $sptk4/acorr -l 9 -m 4 > $tmp/1
    z=$($sptk3/bcut +d -s 0 -e 0 $tmp/1 | $sptk3/x2x +da)
    $sptk3/sopr -d "$z" $tmp/1 > $tmp/2
    $sptk3/nrand -l 10 | $sptk4/acorr -l 9 -m 4 -o 1 > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "acorr: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/acorr -l 10 -m 4 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
