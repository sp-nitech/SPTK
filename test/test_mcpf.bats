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
tmp=test_mcpf

setup() {
    mkdir -p $tmp
}

teardown() {
    rm -rf $tmp
}

@test "mcpf: compatibility" {
    m=9
    M=39
    l=128
    s=2
    a=0.2
    b=0.1

    # From postfiltering_mcp on Training.pl.
    $sptk3/nrand -l 512 |
        $sptk3/mcep -m $m -q 0 -j 5 > $tmp/mc

    $sptk3/step -v 1 -l $s > $tmp/w
    $sptk3/step -v $b -l $((m-s+1)) |
        $sptk3/sopr -a 1 >> $tmp/w

    $sptk3/freqt -m $m -M $M -a $a -A 0 $tmp/mc |
        $sptk3/c2acr -m $M -M 0 -l $l > $tmp/r0

    $sptk3/vopr -m -n $m $tmp/w < $tmp/mc > $tmp/mcw
    $sptk3/freqt -m $m -M $M -a $a -A 0 $tmp/mcw |
        $sptk3/c2acr -m $M -M 0 -l $l > $tmp/r0w

    $sptk3/mc2b -m $m -a $a $tmp/mcw |
        $sptk3/bcp +d -n $m -s 0 -e 0 > $tmp/b0w

    $sptk3/vopr -d < $tmp/r0 $tmp/r0w |
        $sptk3/sopr -LN -d 2 |
        $sptk3/vopr -a $tmp/b0w > $tmp/b0p

    $sptk3/mc2b -m $m -a $a $tmp/mcw |
        $sptk3/bcp +d -n $m -s 1 -e $m |
        $sptk3/merge +d -n $((m-1)) -s 0 -N 0 $tmp/b0p | \
        $sptk3/b2mc -m $m -a $a > $tmp/mcp

    $sptk4/mcpf -m $m -l $l -s $s -a $a -b $b $tmp/mc > $tmp/mcp2
    run $sptk4/aeq $tmp/mcp $tmp/mcp2
    [ "$status" -eq 0 ]
}

@test "mcpf: valgrind" {
    $sptk3/nrand -l 20 > $tmp/1
    run valgrind $sptk4/mcpf -m 9 $tmp/1
    [ "$(echo "${lines[-1]}" | sed -r 's/.*SUMMARY: ([0-9]*) .*/\1/')" -eq 0 ]
}
