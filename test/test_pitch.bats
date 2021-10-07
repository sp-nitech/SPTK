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
tmp=test_pitch
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "pitch: compatibility" {
    # Note -a 3 is not implemented in SPTK3.
    for a in $(seq 0 2); do
        for o in $(seq 0 2); do
            $sptk3/x2x +sd $data | $sptk3/pitch -a "$a" -o "$o" > $tmp/1
            $sptk3/x2x +sd $data | $sptk4/pitch -a "$a" -o "$o" > $tmp/2
            run $sptk4/aeq -e 1 -L $tmp/1 $tmp/2
            [ "$status" -eq 0 ]
        done
    done
}

@test "pitch: valgrind" {
    $sptk3/x2x +sd $data > $tmp/1
    for a in $(seq 0 3); do
        run valgrind $sptk4/pitch -a "$a" $tmp/1
        [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
    done
}
