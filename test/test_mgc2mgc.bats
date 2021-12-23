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
tmp=test_mgc2mgc

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "mgc2mgc: compatibility" {
    $sptk3/nrand -l 20 > $tmp/0

    ary1=("" "-n" "-u" "-n -u")
    ary2=("" "-N" "-U" "-N -U")
    for nu in $(seq 0 3); do
        for NU in $(seq 0 3); do
            # shellcheck disable=SC2086
            $sptk3/mgc2mgc -a 0.1 -A 0.2 -c 2 -C 3 \
                           ${ary1[$nu]} ${ary2[$NU]} $tmp/0 > $tmp/1
            # shellcheck disable=SC2086
            $sptk4/mgc2mgc -a 0.1 -A 0.2 -c 2 -C 3 \
                           ${ary1[$nu]} ${ary2[$NU]} $tmp/0 > $tmp/2
            run $sptk4/aeq $tmp/1 $tmp/2
            [ "$status" -eq 0 ]
        done
    done
}

@test "mgc2mgc: reversibility" {
    $sptk3/nrand -l 16 > $tmp/1
    $sptk4/mgc2mgc -m 7 -a 0.1 -g 0.5 -M 7 -A 0.1 -G 0.5 $tmp/1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "mgc2mgc: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/mgc2mgc -m 4 -M 5 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
