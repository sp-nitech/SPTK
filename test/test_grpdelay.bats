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
tmp=test_grpdelay

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "grpdelay: compatibility" {
    $sptk3/nrand -l 32 > $tmp/1

    # MA
    $sptk3/grpdelay -l 8 $tmp/1 > $tmp/2
    $sptk4/grpdelay -l 8 -z $tmp/1 -m 7 > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]

    # AR
    $sptk3/grpdelay -l 8 -a $tmp/1 > $tmp/2
    $sptk4/grpdelay -l 8 -p $tmp/1 -n 7 > $tmp/3
    run $sptk4/aeq $tmp/2 $tmp/3
    [ "$status" -eq 0 ]
}

@test "grpdelay: valgrind" {
    $sptk3/nrand -l 16 > $tmp/1
    run valgrind $sptk4/grpdelay -l 8 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
