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
tmp=test_resamp
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "resamp: length" {
    $sptk3/x2x +sd $data > $tmp/1
    for a in 0 1 2; do
        $sptk4/resamp -s 16 -S 16 $tmp/1 > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
    done

    len16=$($sptk3/x2x +sa $data | wc -l)
    for S in 8 44.1 48; do
        for a in 0 1 2; do
            expected=$(echo "$len16" | $sptk3/x2x +ad | $sptk4/sopr -m $S -d 16 -CEIL | $sptk3/x2x +da)
            $sptk4/resamp -s 16 -S $S $tmp/1 > $tmp/2
            len=$($sptk3/x2x +da $tmp/2 | wc -l)
            [ "$len" -eq "$expected" ]
        done
    done
}

@test "resamp: valgrind" {
    $sptk3/nrand -l 100 > $tmp/1
    for a in 0 1 2; do
        run valgrind $sptk4/resamp -a $a $tmp/1
        [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
    done
}
