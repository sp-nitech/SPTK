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
tmp=test_iltcdf
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "iltcdf: reversibility" {
    $sptk3/x2x +sd $data | $sptk3/frame -l 400 -p 80 |
        $sptk3/window -l 400 -w 1 -n 1 |
        $sptk3/lpc -l 400 -m 24 |
        $sptk3/lpc2par -m 24 > $tmp/1

    opt=("" "-k")
    for o in $(seq 0 1); do
        # shellcheck disable=SC2086
        $sptk3/nrand -l 19200 |
            $sptk3/ltcdf -m 24 -p 80 ${opt[$o]} $tmp/1 |
            $sptk4/iltcdf -m 24 -p 80 ${opt[$o]} $tmp/1 |
            $sptk3/vstat > $tmp/2
        echo 0 1 | $sptk3/x2x +ad > $tmp/3
        run $sptk4/aeq -t 1e-2 $tmp/2 $tmp/3
        [ "$status" -eq 0 ]
    done
}

@test "iltcdf: valgrind" {
    $sptk3/nrand -l 10 > $tmp/1
    $sptk3/nrand -l 10 > $tmp/2
    run valgrind $sptk4/iltcdf -m 1 -i 0 -p 1 $tmp/1 $tmp/2
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
