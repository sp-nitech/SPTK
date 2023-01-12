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
tmp=test_excite

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "excite: compatibility" {
    # There is no compatibility with SPTK3.
    # The values within only unvoiced region is checked.
    $sptk3/step -l 10 -v 0 > $tmp/1
    $sptk3/excite -p 4 $tmp/1 > $tmp/2
    $sptk4/excite -p 4 $tmp/1 > $tmp/3
    run $sptk4/aeq -L $tmp/2 $tmp/3
    [ "$status" -eq 0 ]

    $sptk3/excite -n -p 4 $tmp/1 > $tmp/2
    $sptk4/excite -n 1 -p 4 $tmp/1 > $tmp/3
    run $sptk4/aeq -L $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "excite: valgrind" {
    $sptk3/ramp -l 10 > $tmp/1
    run valgrind $sptk4/excite -p 2 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
