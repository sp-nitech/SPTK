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
tmp=test_lspdf
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

# Note `lspdf -i 0` has no compatibility due to the change of implementation.
@test "lspdf: compatibility" {
    ary1=("" "-L" "")
    ary2=("" "-L" "-k")
    for k in $(seq 0 2); do
        # shellcheck disable=SC2086
        $sptk3/x2x +sd $data | $sptk3/frame -l 400 -p 80 |
            $sptk3/window -l 400 -w 1 -n 1 |
            $sptk3/lpc -l 400 -m 12 |
            $sptk3/lpc2lsp -m 12 ${ary1[$k]} > $tmp/1
        # shellcheck disable=SC2086
        $sptk3/nrand -l 19200 | $sptk3/lspdf -m 12 -p 80 ${ary2[$k]} $tmp/1 > $tmp/2
        $sptk3/nrand -l 19200 | $sptk4/lspdf -m 12 -p 80 -k "$k" $tmp/1 > $tmp/3
        run $sptk4/aeq -L $tmp/2 $tmp/3
        [ "$status" -eq 0 ]
    done
}

@test "lspdf: identity" {
    $sptk3/step -l 10 > $tmp/1
    $sptk3/nrand -l 10 > $tmp/2
    $sptk4/lspdf -m 0 -i 0 -p 1 $tmp/1 $tmp/2 > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "lspdf: valgrind" {
    $sptk3/nrand -l 10 > $tmp/1
    $sptk3/nrand -l 10 > $tmp/2
    run valgrind $sptk4/lspdf -m 1 -i 0 -p 1 $tmp/1 $tmp/2
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
