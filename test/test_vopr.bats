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
tmp=test_vopr

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "vopr: compatibility" {
    ary1=("-a" "-s" "-m" "-d"
          "-ATAN" "-AM" "-GM" "-c" "-f"
          "-gt" "-ge" "-lt" "-le" "-eq" "-ne")
    ary2=("-a" "-s" "-m" "-d"
          "-ATAN" "-AM" "-GM" "-MIN" "-MAX"
          "-GT" "-GE" "-LT" "-LE" "-EQ" "-NE")
    $sptk3/nrand -l 12 -s 1 | $sptk3/sopr -ABS > $tmp/1
    $sptk3/nrand -l 12 -s 1 | $sptk3/sopr -ABS > $tmp/2
    for i in $(seq 0 $((${#ary1[@]} - 1))); do
        $sptk3/vopr $tmp/1 $tmp/2 "${ary1[$i]}" > $tmp/3
        $sptk4/vopr $tmp/1 $tmp/2 "${ary2[$i]}" -q 0 > $tmp/4
        run $sptk4/aeq $tmp/3 $tmp/4
        [ "$status" -eq 0 ]

        $sptk3/vopr $tmp/1 $tmp/2 "${ary1[$i]}" -l 2 > $tmp/3
        $sptk4/vopr $tmp/1 $tmp/2 "${ary2[$i]}" -l 2 -q 1 > $tmp/4
        run $sptk4/aeq $tmp/3 $tmp/4
        [ "$status" -eq 0 ]

        $sptk3/vopr $tmp/1 "${ary1[$i]}" -l 2 -i > $tmp/3
        $sptk4/vopr $tmp/1 "${ary2[$i]}" -l 2 -q 2 > $tmp/4
        run $sptk4/aeq $tmp/3 $tmp/4
        [ "$status" -eq 0 ]
    done
}

@test "vopr: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/vopr -a $tmp/1 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
