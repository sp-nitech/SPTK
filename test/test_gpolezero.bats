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
tmp=test_gpolezero

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "gpolezero: running" {
    . ./tools/venv/bin/activate
    echo 3 2 1 | $sptk3/x2x +ad | $sptk3/root_pol -m 2 > $tmp/1
    $sptk4/gpolezero -z $tmp/1 $tmp/2.png
    deactivate
}
