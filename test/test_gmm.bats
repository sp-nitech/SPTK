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
tmp=test_gmm

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "gmm: compatibility" {
    # Note that there is no compatibility without diagonal covariance.
    # This is because SPTK3 use previous mean instead of current mean
    # to update covariance.
    $sptk3/nrand -s 1 -l 256 | $sptk3/gmm -l 4 -m 4 -b 19 > $tmp/1
    $sptk3/nrand -s 1 -l 256 | $sptk4/gmm -l 4 -k 4 -i 20 > $tmp/2

    $sptk3/bcp +d -l 9 -s 0 -e 0 $tmp/2 > $tmp/2.w
    $sptk3/bcp +d -l 9 -s 1 $tmp/2 > $tmp/2.mv
    cat $tmp/2.w $tmp/2.mv > $tmp/3
    run $sptk4/aeq $tmp/1 $tmp/3
    [ "$status" -eq 0 ]

    # MAP estimate.
    $sptk3/nrand -s 2 -l 256 |
        $sptk3/gmm -l 4 -m 4 -b 19 -F $tmp/1 -M 0.1 > $tmp/4
    $sptk3/nrand -s 2 -l 256 |
        $sptk4/gmm -l 4 -k 4 -i 20 -U $tmp/2 -M 0.1 > $tmp/5

    $sptk3/bcp +d -l 9 -s 0 -e 0 $tmp/5 > $tmp/5.w
    $sptk3/bcp +d -l 9 -s 1 $tmp/5 > $tmp/5.mv
    cat $tmp/5.w $tmp/5.mv > $tmp/6
    run $sptk4/aeq $tmp/4 $tmp/6
    [ "$status" -eq 0 ]
}

@test "gmm: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/gmm -l 2 -k 2 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
