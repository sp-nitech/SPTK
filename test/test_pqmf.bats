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
tmp=test_pqmf
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "pqmf: reversibility" {
    k=4
    $sptk3/x2x +sd $data > $tmp/1
    for m in 49 50; do
        $sptk4/pqmf -k $k -m $m $tmp/1 |
            $sptk4/ipqmf -k $k -m $m > $tmp/2

        $sptk3/vopr -s $tmp/1 $tmp/2 |
            $sptk3/sopr -ABS |
            $sptk3/average > $tmp/3

        err=$($sptk3/sopr -FIX $tmp/3 | $sptk3/x2x +da)
        [ "$err" -lt 5 ]
    done
}

@test "pqmf: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/pqmf -k 2 -m 10 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
