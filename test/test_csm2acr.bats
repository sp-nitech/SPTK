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
tmp=test_csm2acr

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "csm2acr: compatibility" {
    $sptk3/nrand -l 20 | $sptk3/csm2acr -m 5 > $tmp/1
    $sptk3/nrand -l 20 | $sptk4/csm2acr -m 9 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "csm2acr: reversibility" {
    # Make fake CSM parameters.
    $sptk3/ramp -s 0.314 -l 5 -t 0.628 > $tmp/1
    $sptk3/nrand -l 5 >> $tmp/1
    $sptk4/csm2acr -m 9 $tmp/1 | $sptk4/acr2csm -m 9 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "csm2acr: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/csm2acr -m 9 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
