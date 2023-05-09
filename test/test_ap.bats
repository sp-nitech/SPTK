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
tmp=test_ap
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "ap: valgrind" {
    $sptk3/x2x +sd $data > $tmp/0
    $sptk4/pitch $tmp/0 > $tmp/1
    for a in $(seq 0 1); do
        run valgrind $sptk4/ap -a "$a" -l 16 $tmp/1 $tmp/0
        [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
    done
}
