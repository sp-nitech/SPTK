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
tmp=test_nan

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "nan: compatibility" {
    $sptk3/nrand -l 10 | $sptk3/nan > $tmp/1
    $sptk3/nrand -l 10 | $sptk4/nan > $tmp/2
    run diff -q $tmp/1 $tmp/2
    [ "${lines[0]}" = "" ]
    # Generate NaN and Inf and check them.
    $sptk3/ramp -l 2 | $sptk3/sopr -d 0 | $sptk3/nan > $tmp/1
    $sptk3/ramp -l 2 | $sptk3/sopr -d 0 | $sptk4/nan > $tmp/2
    run diff -q $tmp/1 $tmp/2
    [ "${lines[0]}" = "" ]
}

@test "nan: valgrind" {
    $sptk3/nrand -l 10 > $tmp/1
    run valgrind $sptk4/nan $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
