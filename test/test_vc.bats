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
tmp=test_vc

setup() {
    mkdir -p $tmp

    # Make training data.
    $sptk3/nrand -s 1 -l 300 -m 1 -v 0.2 |
        $sptk3/delta -d -0.5 0 0.5 -l 3 > $tmp/0.s
    $sptk3/nrand -s 2 -l 200 -m -1 -v 0.1 |
        $sptk3/delta -d -0.5 0 0.5 -l 2 > $tmp/0.t
    $sptk3/merge +d $tmp/0.s $tmp/0.t -L 6 -l 4 > $tmp/0
}

teardown() {
    rm -rf $tmp
}

@test "vc: compatibility" {
    # Make GMM.
    $sptk3/gmm $tmp/0 -l 10 -B 10 -m 4 > $tmp/1

    # Rearrange the GMM to ensure the compatibility with SPTK4.
    $sptk3/bcut +d -e 3 $tmp/1 > $tmp/1.w
    $sptk3/bcut +d -s 4 $tmp/1 | $sptk3/sopr -magic 0 > $tmp/1.mv
    $sptk3/merge +d $tmp/1.w $tmp/1.mv -L 1 -l 20 > $tmp/2

    # Make source.
    $sptk3/nrand -s 3 -l 100 -m 1 -v 0.2 > $tmp/3

    # Make target.
    $sptk3/vc $tmp/1 -l 3 -L 2 -m 4 -d -0.5 0 0.5 < $tmp/3 > $tmp/4
    $sptk3/delta -d -0.5 0 0.5 -l 3 $tmp/3 |
        $sptk4/vc $tmp/2 -l 3 -L 2 -k 4 -d -0.5 0 0.5 > $tmp/5
    run $sptk4/aeq $tmp/4 $tmp/5
    [ "$status" -eq 0 ]

    # Make target.
    $sptk3/vc $tmp/1 -l 3 -L 2 -m 4 -r 1 1 < $tmp/3 > $tmp/6
    $sptk3/delta -r 1 1 -l 3 $tmp/3 |
        $sptk4/vc $tmp/2 -l 3 -L 2 -k 4 -r 1 > $tmp/7
    run $sptk4/aeq $tmp/6 $tmp/7
    [ "$status" -eq 0 ]
}

@test "vc: block covariance" {
    # Make GMM.
    $sptk4/gmm $tmp/0 -l 10 -B 6 4 -k 4 -f > $tmp/1
    $sptk4/gmm $tmp/0 -l 10 -k 4 > $tmp/2

    # Make source.
    $sptk3/nrand -s 3 -l 30 -m 1 -v 0.2 |
        $sptk3/delta -d -0.5 0 0.5 -l 3 > $tmp/3

    # Make target.
    $sptk4/vc $tmp/1 -l 3 -L 2 -k 4 -d -0.5 0 0.5 -f < $tmp/3 > $tmp/4
    $sptk4/vc $tmp/2 -l 3 -L 2 -k 4 -d -0.5 0 0.5 < $tmp/3 > $tmp/5
    run $sptk4/aeq $tmp/4 $tmp/5 -e 1 -t 1
    [ "$status" -eq 0 ]
}

@test "vc: valgrind" {
    $sptk4/gmm $tmp/0 -l 10 -k 2 > $tmp/1
    $sptk3/nrand -l 20 | $sptk3/delta -d -0.5 0 0.5 -l 3 > $tmp/2
    run valgrind $sptk4/vc $tmp/1 $tmp/2 -l 3 -L 2 -k 2 -d -0.5 0 0.5
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
