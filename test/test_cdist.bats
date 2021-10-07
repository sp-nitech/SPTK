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
tmp=test_cdist

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "cdist: compatibility" {
    $sptk3/nrand -s 123 -l 50 > $tmp/1
    $sptk3/nrand -s 321 -l 50 > $tmp/2
    ary=(0 2 1)
    for o in $(seq 0 2); do
        $sptk3/cdist -f -m 4 -o "${ary[$o]}" $tmp/1 $tmp/2 > $tmp/3
        $sptk4/cdist -f -m 4 -o "$o" $tmp/1 $tmp/2 > $tmp/4
        run $sptk4/aeq $tmp/3 $tmp/4
        [ "$status" -eq 0 ]
    done

    $sptk3/cdist -m 4 $tmp/1 $tmp/2 > $tmp/3
    $sptk4/cdist -m 4 $tmp/1 $tmp/2 > $tmp/4
    run $sptk4/aeq $tmp/3 $tmp/4
    [ "$status" -eq 0 ]
}

@test "cdist: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/cdist -m 4 $tmp/1 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
