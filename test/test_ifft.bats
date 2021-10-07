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
tmp=test_ifft

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "ifft: compatibility" {
    ary=("" "-R" "-I")
    for o in $(seq 0 2); do
        # shellcheck disable=SC2086
        $sptk3/nrand -l 32 | $sptk3/ifft -l 8 ${ary[$o]} > $tmp/1
        $sptk3/nrand -l 32 | $sptk4/ifft -l 8 -o "$o" > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done
}

@test "ifft: reversibility" {
    $sptk3/nrand -l 16 > $tmp/1
    $sptk4/ifft -l 8 $tmp/1 | $sptk4/fft -l 8 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "ifft: valgrind" {
    $sptk3/nrand -l 32 > $tmp/1
    run valgrind $sptk4/ifft -l 8 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
