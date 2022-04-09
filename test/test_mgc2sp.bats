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
tmp=test_mgc2sp

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "mgc2sp: compatibility" {
    $sptk3/step -v 0.1 -l 16 > $tmp/0
    ary=("-o 0" "-o 1" "-o 2" "-o 3" "-p -o 0" "-p -o 1" "-p -o 2")
    for nu in "" "-n" "-u" "-n -u"; do
        for o in $(seq 0 6); do
            # shellcheck disable=SC2086
            $sptk3/mgc2sp -m 15 -l 32 -a 0.1 -c 2 ${ary[$o]} $nu $tmp/0 > $tmp/1
            # shellcheck disable=SC2086
            $sptk4/mgc2sp -m 15 -l 32 -a 0.1 -c 2 -o "$o" $nu $tmp/0 > $tmp/2
            run $sptk4/aeq $tmp/1 $tmp/2
            [ "$status" -eq 0 ]
        done
    done
}

@test "mgc2sp: valgrind" {
    $sptk3/nrand -l 32 > $tmp/1
    run valgrind $sptk4/mgc2sp -m 15 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
