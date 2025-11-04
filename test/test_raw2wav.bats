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

sptk4=bin
tmp=test_raw2wav
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "raw2wav: compatibility" {
    # short
    $sptk4/raw2wav +s $data > $tmp/short.wav
    ffmpeg -y -f s16le -ar 16000 -ac 1 -i $data $tmp/target.wav
    run diff $tmp/short.wav $tmp/target.wav

    # float
    $sptk4/x2x +sf $data | $sptk4/sopr -d 32768 > $tmp/float.raw
    $sptk4/raw2wav +f $data > $tmp/float.wav
    ffmpeg -y -f f32le -ar 16000 -ac 1 -i $tmp/float.raw $tmp/target.wav
    run diff $tmp/float.wav $tmp/target.wav
}

@test "raw2wav: valgrind" {
    run valgrind $sptk4/raw2wav +s $data > data.wav
}
