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
tmp=test_fftcep

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "fftcep: compatibility" {
    for i in 0 3; do
        $sptk3/nrand -l 32 | $sptk3/fftcep -l 16 -m 4 -j "$i" -k 1 > $tmp/1
        $sptk3/nrand -l 32 | $sptk4/fftcep -l 16 -m 4 -i "$i" -a 1 > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done
}

@test "fftcep: valgrind" {
    $sptk3/nrand -l 32 > $tmp/1
    run valgrind $sptk4/fftcep -l 16 -m 4 -i 3 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
