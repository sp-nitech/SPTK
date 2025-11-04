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
tmp=test_wav2raw
data=asset/data.short

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "wav2raw: compatibility" {
    # short
    for fmt in wav flac mp3 ogg; do
        ffmpeg -y -f s16le -ar 16000 -ac 1 -i $data $tmp/input.$fmt
        ffmpeg -y -i $tmp/input.$fmt -f s16le $tmp/input.raw
        x2x +sd $tmp/input.raw > $tmp/target.raw
        $sptk4/wav2raw +s $tmp/input.$fmt | $sptk4/x2x +sd > $tmp/output.raw
        if [ $fmt = "wav" ] || [ $fmt = "flac" ]; then
            run $sptk4/aeq $tmp/target.raw $tmp/output.raw
            [ "$status" -eq 0 ]
        elif [ $fmt = "mp3" ] || [ $fmt = "ogg" ]; then
            $sptk4/vopr -s $tmp/target.raw $tmp/output.raw | $sptk4/sopr -ABS > $tmp/diff
            $sptk4/impulse -l 1 > $tmp/one
            $sptk4/vopr -q 1 -GT $tmp/diff $tmp/one | $sptk4/vsum | $sptk4/x2x +da > $tmp/error
            n=$(cat $tmp/error)
            [ "$n" -eq 0 ]
        fi
    done

    # float
    for fmt in wav flac mp3 ogg; do
        ffmpeg -y -f s16le -ar 16000 -ac 1 -i $data $tmp/input.$fmt
        ffmpeg -y -i $tmp/input.$fmt -f f64le $tmp/target.raw
        $sptk4/wav2raw +f $tmp/input.$fmt | $sptk4/x2x +fd > $tmp/output.raw
        run $sptk4/aeq $tmp/target.raw $tmp/output.raw
        [ "$status" -eq 0 ]
    done

    # stereo
    for fmt in wav flac mp3 ogg; do
        ffmpeg -y -i $tmp/input.$fmt -ac 2 $tmp/stereo_input.$fmt
        ffmpeg -y -i $tmp/stereo_input.$fmt -f f64le $tmp/target.raw
        $sptk4/wav2raw +f $tmp/stereo_input.$fmt | $sptk4/x2x +fd > $tmp/output.raw
        run $sptk4/aeq $tmp/target.raw $tmp/output.raw
        [ "$status" -eq 0 ]
    done
}

@test "wav2raw: valgrind" {
    for fmt in wav flac mp3 ogg; do
        ffmpeg -y -f s16le -ar 16000 -ac 1 -i $data $tmp/input.$fmt
        run valgrind $sptk4/wav2raw +s $tmp/input.$fmt > /dev/null
        [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
    done
}
