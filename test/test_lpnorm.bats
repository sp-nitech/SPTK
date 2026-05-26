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
tmp=test_lpnorm

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "lpnorm: compatibility" {
    for p in 1 2 inf; do
        case $p in
            inf) ord="float('inf')" ;;
            *) ord="$p" ;;
        esac
        cmd="from scipy.linalg import norm; "
        cmd+="x = [0.1, 0.2, 0.3, 0.4]; "
        cmd+="n = norm(x, ord=$ord); "
        cmd+="y = [i / n for i in x]; "
        cmd+="print(*y)"
        tools/venv/bin/python -c "${cmd}" | $sptk3/x2x +ad > $tmp/1
        $sptk3/ramp -s 0.1 -t 0.1 -l 4 | $sptk4/lpnorm -l 4 -p $p > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done
}

@test "lpnorm: large p value" {
    $sptk3/step -l 1 -v 1 > $tmp/1
    $sptk3/nrand -l 1 | $sptk4/lpnorm -l 1 -p 1000 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "lpnorm: valgrind" {
    $sptk3/ramp -l 10 > $tmp/1
    run valgrind $sptk4/lpnorm -l 10 -f $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
