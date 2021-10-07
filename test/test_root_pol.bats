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
tmp=test_root_pol

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "root_pol: compatibility" {
    ary1=("" "-s" "-r")
    ary2=("-q 0 -o 0" "-q 1" "-o 1")
    for i in $(seq 0 2); do
        # shellcheck disable=SC2086
        $sptk3/nrand -l 32 | $sptk3/root_pol -m 32 ${ary1[$i]} |
            $sptk3/x2x +da2 %.16f | sort -k 1,1n 2,2n | $sptk3/x2x +ad > $tmp/1
        # shellcheck disable=SC2086
        $sptk3/nrand -l 32 | $sptk4/root_pol -m 31 ${ary2[$i]} |
            $sptk3/x2x +da2 %.16f | sort -k 1,1n 2,2n | $sptk3/x2x +ad > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done
}

@test "root_pol: valgrind" {
    $sptk3/nrand -l 32 > $tmp/1
    run valgrind $sptk4/root_pol -m 31 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
