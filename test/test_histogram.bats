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
tmp=test_histogram

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "histogram: compatibility" {
    # There is no compatibility with SPTK3 but numpy.histogram.
    cmd="import numpy as np; "
    cmd+="h, _ = np.histogram(np.arange(10), bins=4, range=(0, 9)); "
    cmd+="print(' '.join(map(str, h)))"
    tools/venv/bin/python -c "${cmd}" | $sptk3/x2x +ad > $tmp/1
    $sptk3/ramp -l 10 | $sptk4/histogram -l 0 -u 9 -b 4 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]

    cmd="import numpy as np; "
    cmd+="h, e = np.histogram(np.arange(10), bins=4, range=(0, 9), density=True); "
    cmd+="h *= np.diff(e); "
    cmd+="print(' '.join(map(str, h)))"
    tools/venv/bin/python -c "${cmd}" | $sptk3/x2x +ad > $tmp/3
    $sptk3/ramp -l 10 | $sptk4/histogram -l 0 -u 9 -b 4 -n > $tmp/4
    run $sptk4/aeq $tmp/3 $tmp/4
    [ "$status" -eq 0 ]
}

@test "histogram: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/histogram -l 10 -b 2 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
