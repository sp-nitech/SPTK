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
tmp=test_mlsacheck

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "mlsacheck: compatibility" {
    $sptk3/nrand -l 32 |
        $sptk3/mcep -l 16 -m 4 -a 0.1 |
        $sptk3/sopr -m 10 > $tmp/1

    ary1=("-c 2" "-c 3" "-c 4")
    ary2=("-t 0" "-t 1" "-f")
    for i in $(seq 0 2); do
        for r in $(seq 0 1); do
            for p in $(seq 4 7); do
                # shellcheck disable=SC2086
                $sptk3/mlsacheck -m 4 -a 0.1 ${ary1[$i]} -r "$r" -P "$p" \
                                 $tmp/1 > $tmp/2
                # shellcheck disable=SC2086
                $sptk4/mlsacheck -m 4 -a 0.1 ${ary2[$i]} -r "$r" -P "$p" -x \
                                 $tmp/1 > $tmp/3
                run $sptk4/aeq $tmp/2 $tmp/3
                [ "$status" -eq 0 ]
            done
        done
    done
}

@test "mlsacheck: identity" {
    $sptk3/nrand -l 32 | $sptk3/mcep -l 16 -m 4 -a 0.1 > $tmp/1
    $sptk4/mlsacheck -m 4 $tmp/1 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "mlsacheck: valgrind" {
    $sptk3/nrand -l 32 | $sptk3/mcep -l 16 -m 4 -a 0.1 > $tmp/1
    run valgrind $sptk4/mlsacheck -m 4 -x $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
