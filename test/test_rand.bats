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
tmp=test_rand

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "rand: compatibility" {
    echo -1 1 | $sptk3/x2x +ad > $tmp/1
    $sptk4/rand -l 512 -a -1 -b 1 -s 123 | $sptk3/minmax > $tmp/2
    run $sptk4/aeq -e 1 -t 0.01 $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "rand: valgrind" {
    run valgrind $sptk4/rand -l 10
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
