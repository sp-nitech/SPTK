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
tmp=test_lpc2lsp

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "lpc2lsp: compatibility" {
    $sptk3/nrand -l 800 | $sptk3/lpc -l 400 -m 12 > $tmp/1
    ary=("" "-L" "-k")
    for k in $(seq 0 2); do
        # shellcheck disable=SC2086
        $sptk3/lpc2lsp -m 12 ${ary[$k]} -n 256 $tmp/1 > $tmp/2
        $sptk4/lpc2lsp -m 12 -k "$k" -n 256 $tmp/1 > $tmp/3
        run $sptk4/aeq $tmp/2 $tmp/3
        [ "$status" -eq 0 ]
    done

    for o in $(seq 0 3); do
        $sptk3/lpc2lsp -m 12 -o "$o" -n 256 $tmp/1 > $tmp/2
        $sptk4/lpc2lsp -m 12 -o "$o" -n 256 $tmp/1 > $tmp/3
        run $sptk4/aeq $tmp/2 $tmp/3
        [ "$status" -eq 0 ]
    done

    $sptk3/nrand -l 800 | $sptk3/lpc -l 400 -m 11 > $tmp/1
    $sptk3/lpc2lsp -m 11 -n 256 $tmp/1 > $tmp/2
    $sptk4/lpc2lsp -m 11 -n 256 $tmp/1 > $tmp/3
}

@test "lpc2lsp: reversibility" {
    $sptk3/nrand -l 400 | $sptk3/lpc -l 400 -m 12 > $tmp/1
    $sptk4/lpc2lsp -m 12 $tmp/1 | $sptk4/lsp2lpc -m 12 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "lpc2lsp: valgrind" {
    $sptk3/nrand -l 800 | $sptk3/lpc -l 400 -m 12 > $tmp/1
    run valgrind $sptk4/lpc2lsp -m 12 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
