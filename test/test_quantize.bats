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
tmp=test_quantize

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "quantize: reversibility" {
    $sptk3/nrand -l 10 -v 10 > $tmp/0
    for t in $(seq 0 1); do
        for o in $(seq 0 1); do
            $sptk4/quantize $tmp/0 -b 4 -v 16 -t "$t" -o "$o" > $tmp/1
            $sptk4/dequantize -b 4 -v 16 -t "$t" -q "$o" $tmp/1 |
                $sptk4/quantize -b 4 -v 16 -t "$t" -o "$o" > $tmp/2
            run $sptk4/aeq $tmp/1 $tmp/2
            [ "$status" -eq 0 ]
        done
    done
}

@test "quantize: valgrind" {
    $sptk3/nrand -l 10 > $tmp/1
    run valgrind $sptk4/quantize -b 2 -v 2 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
