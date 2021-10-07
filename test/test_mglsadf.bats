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
tmp=test_mglsadf
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

# Note `mglsadf -i 0` has no compatibility due to the change of implementation.
@test "mglsadf: compatibility (c = 0)" {
    $sptk3/x2x +sd $data | $sptk3/frame -l 400 -p 80 |
        $sptk3/window -l 400 -L 512 -w 1 -n 1 |
        $sptk3/mcep -l 512 -m 24 > $tmp/1

    opt=("" "-t" "-k")
    for o in $(seq 0 2); do
        # shellcheck disable=SC2086
        $sptk3/nrand -l 19200 |
            $sptk3/mlsadf -m 24 -p 80 -P 7 ${opt[$o]} $tmp/1 > $tmp/2
        # shellcheck disable=SC2086
        $sptk3/nrand -l 19200 |
            $sptk4/mglsadf -m 24 -p 80 -P 7 ${opt[$o]} $tmp/1 > $tmp/3
        run $sptk4/aeq -L $tmp/2 $tmp/3
        [ "$status" -eq 0 ]
    done
}

@test "mglsadf: compatibility (c > 0)" {
    $sptk3/x2x +sd $data | $sptk3/frame -l 400 -p 80 |
        $sptk3/window -l 400 -L 512 -w 1 -n 1 |
        $sptk3/mgcep -l 512 -m 24 -c 2 > $tmp/1

    opt=("" "-t" "-k")
    for o in $(seq 0 2); do
        # shellcheck disable=SC2086
        $sptk3/nrand -l 19200 |
            $sptk3/mglsadf -m 24 -p 80 -c 2 ${opt[$o]} $tmp/1 > $tmp/2
        # shellcheck disable=SC2086
        $sptk3/nrand -l 19200 |
            $sptk4/mglsadf -m 24 -p 80 -c 2 ${opt[$o]} $tmp/1 > $tmp/3
        run $sptk4/aeq -L $tmp/2 $tmp/3
        [ "$status" -eq 0 ]
    done
}

@test "mglsadf: valgrind" {
    $sptk3/nrand -l 10 > $tmp/1
    $sptk3/nrand -l 10 > $tmp/2
    run valgrind $sptk4/mglsadf -m 1 -i 0 -p 1 $tmp/1 $tmp/2
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
    run valgrind $sptk4/mglsadf -m 1 -i 0 -p 1 -c 1 $tmp/1 $tmp/2
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
