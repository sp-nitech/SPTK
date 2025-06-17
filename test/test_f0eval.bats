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
tmp=test_f0eval
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "f0eval: compatibility (f0 rmse)" {
    $sptk3/x2x +sd $data | $sptk3/pitch -a 0 -o 2 > $tmp/1
    $sptk3/x2x +sd $data | $sptk3/pitch -a 1 -o 2 > $tmp/2
    $sptk3/sopr -magic -1e+10 -m cent -MAGIC -1e+10 $tmp/1 > $tmp/3
    $sptk3/sopr -magic -1e+10 -m cent -MAGIC -1e+10 $tmp/2 > $tmp/4
    $sptk4/rmse -magic -1e+10 $tmp/3 $tmp/4 > $tmp/5
    $sptk4/f0eval -q 2 -o 1 $tmp/1 $tmp/2 > $tmp/6
    run $sptk4/aeq $tmp/5 $tmp/6
    [ "$status" -eq 0 ]
}

@test "f0eval: compatibility (vuv error)" {
    $sptk3/x2x +sd $data | $sptk3/pitch -a 0 -o 1 > $tmp/1
    $sptk3/x2x +sd $data | $sptk3/pitch -a 1 -o 1 > $tmp/2
    $sptk4/sopr -SIGN $tmp/1 > $tmp/3
    $sptk4/sopr -SIGN $tmp/2 > $tmp/4
    $sptk4/vopr -NE $tmp/3 $tmp/4 | $sptk4/vsum > $tmp/5
    n=$($sptk3/x2x +da $tmp/1 | wc -l)
    $sptk4/sopr -d "$n" -m 100 $tmp/5 > $tmp/6
    $sptk4/f0eval -q 1 -o 2 $tmp/1 $tmp/2 > $tmp/7
    run $sptk4/aeq $tmp/6 $tmp/7
    [ "$status" -eq 0 ]
}

@test "f0eval: input format" {
    $sptk3/x2x +sd $data | $sptk3/pitch -a 0 -o 0 > $tmp/1
    $sptk3/x2x +sd $data | $sptk3/pitch -a 1 -o 0 > $tmp/2
    $sptk4/f0eval -q 1 $tmp/1 $tmp/2 > $tmp/3
    $sptk3/x2x +sd $data | $sptk3/pitch -a 0 -o 1 > $tmp/1
    $sptk3/x2x +sd $data | $sptk3/pitch -a 1 -o 1 > $tmp/2
    $sptk4/f0eval -q 1 $tmp/1 $tmp/2 > $tmp/4
    $sptk3/x2x +sd $data | $sptk3/pitch -a 0 -o 2 > $tmp/1
    $sptk3/x2x +sd $data | $sptk3/pitch -a 1 -o 2 > $tmp/2
    $sptk4/f0eval -q 2 $tmp/1 $tmp/2 > $tmp/5
    run $sptk4/aeq $tmp/3 $tmp/4
    [ "$status" -eq 0 ]
    run $sptk4/aeq $tmp/3 $tmp/5
    [ "$status" -eq 0 ]
}

@test "f0eval: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/f0eval $tmp/1 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
