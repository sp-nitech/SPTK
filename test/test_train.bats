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
tmp=test_train

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "train: compatibility (p = 1)" {
    for n in $(seq 0 2); do
        $sptk3/train -l 10 -n "$n" -p 1 > $tmp/1
        $sptk4/train -l 10 -n "$n" -p 1 > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done
}

@test "train: compatibility (p > 1)" {
    $sptk3/step -v 3.14 -l 2 | $sptk4/excite -p 100 > $tmp/1
    $sptk4/train -n 1 -p 3.14 -m 100 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "train: valgrind" {
    run valgrind $sptk4/train -l 10
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
