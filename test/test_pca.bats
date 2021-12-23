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
tmp=test_pca

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "pca: compatibility" {
    $sptk3/nrand -l 1024 | $sptk3/pca -l 16 -n 4 -v -V $tmp/3 > $tmp/1
    $sptk3/nrand -l 1024 | $sptk4/pca -l 16 -n 4 -v $tmp/4 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
    $sptk3/bcp +d -s 0 -e 0 -l 2 $tmp/3 > $tmp/5
    $sptk3/bcp +d -s 1 -e 1 -l 2 $tmp/3 > $tmp/6
    cat $tmp/5 $tmp/6 > $tmp/7
    run $sptk4/aeq $tmp/7 $tmp/4
    [ "$status" -eq 0 ]
}

@test "pca: valgrind" {
    $sptk3/nrand -l 32 > $tmp/1
    run valgrind $sptk4/pca -l 4 -n 2 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
