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
tmp=test_medfilt

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "medfilt: compatibility" {
    cmd="from scipy.signal import medfilt; "
    cmd+="y = medfilt([0, 1, -2, 7, 4, 8, -5, -6, 3], kernel_size=3); "
    cmd+="print(' '.join(map(str, y[1:-1])))"
    tools/venv/bin/python -c "${cmd}" | $sptk3/x2x +ad > $tmp/1
    echo "0 1 -2 7 4 8 -5 -6 3" | $sptk3/x2x +ad |
        $sptk4/medfilt -m 0 -k 2 | $sptk3/bcut +d -s 1 -e 7 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "medfilt: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/histogram -l 2 -k 2 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
