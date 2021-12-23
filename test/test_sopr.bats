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
tmp=test_sopr

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "sopr: compatibility" {
    $sptk3/nrand -l 10 | $sptk3/sopr -ABS > $tmp/0

    ary1=(" " "-a 1" "-s 1" "-m 2" "-d 2" "-p 2" "-f 0" "-c 0"
          "-ABS" "-INV" "-P" "-R"
          "-LN" "-LOG2" "-LOG10" "-LOGX 20"
          "-EXP" "-POW2" "-POW10" "-POWX 20"
          "-FIX" "-UNIT" "-CLIP"
          "-SIN" "-COS" "-TAN" "-ATAN")
    ary2=(" " "-a 1" "-s 1" "-m 2" "-d 2" "-p 2" "-l 0" "-u 0"
          "-ABS" "-INV" "-SQR" "-SQRT"
          "-LN" "-LOG2" "-LOG10" "-LOGX 20"
          "-EXP" "-POW2" "-POW10" "-POWX 20"
          "-ROUND" "-UNIT" "-RAMP"
          "-SIN" "-COS" "-TAN" "-ATAN")
    for i in $(seq 0 $((${#ary1[@]} - 1))); do
        # shellcheck disable=SC2086
        $sptk3/sopr $tmp/0 ${ary1[$i]} > $tmp/1
        # shellcheck disable=SC2086
        $sptk4/sopr $tmp/0 ${ary2[$i]} > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done

    ary3=("pi" "dB" "cent" "semitone" "octave" "sqrt100" "ln10" "exp1")
    for i in $(seq 0 $((${#ary3[@]} - 1))); do
        $sptk3/sopr $tmp/0 -m "${ary3[$i]}" > $tmp/1
        $sptk4/sopr $tmp/0 -m "${ary3[$i]}" > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done

    ary4=("-magic 0" "-magic 0 -MAGIC -1")
    for i in $(seq 0 $((${#ary4[@]} - 1))); do
        # shellcheck disable=SC2086
        $sptk3/ramp -l 3 | $sptk3/sopr ${ary4[$i]} > $tmp/1
        # shellcheck disable=SC2086
        $sptk3/ramp -l 3 | $sptk4/sopr ${ary4[$i]} > $tmp/2
        run $sptk4/aeq $tmp/1 $tmp/2
        [ "$status" -eq 0 ]
    done
    $sptk3/ramp -l 3 > $tmp/1
    $sptk4/sopr $tmp/1 -magic 0 -MAGIC -1 -magic -1 -MAGIC 0 > $tmp/2
    run $sptk4/aeq $tmp/1 $tmp/2
    [ "$status" -eq 0 ]
}

@test "sopr: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/sopr -m 2 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
