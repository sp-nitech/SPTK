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

import struct
import sys

import numpy as np

line_styles = ["solid", "dash", "dot", "dashdot"]

marker_symbols = {
    0: None,
    1: "circle-open-dot",
    2: "x-open-dot",
    3: "square-open",
    4: "triangle-up-open",
    5: "circle-open",
    6: "diamond-open",
    7: "x-thin",
    8: "cross-thin",
    9: "circle-x-open",
    10: "circle-cross-open",
    11: "square",
    12: "triangle-up",
    13: "circle",
    14: "diamond",
    15: "asterisk",
}

sptk2numpy = {
    "c": np.int8,
    "C": np.uint8,
    "s": np.int16,
    "S": np.uint16,
    "i": np.int32,
    "I": np.uint32,
    "l": np.int64,
    "L": np.uint64,
    "f": np.float32,
    "d": np.float64,
    "e": np.float128,
}

sptk2struct = {
    "c": ("b", 1),
    "C": ("B", 1),
    "s": ("h", 2),
    "S": ("H", 2),
    "i": ("i", 4),
    "I": ("I", 4),
    "l": ("q", 8),
    "L": ("Q", 8),
    "f": ("f", 4),
    "d": ("d", 8),
}


def asarray(data, dim=None, dtype="d"):
    data = np.asarray(data, dtype=sptk2numpy[dtype])
    if dim is None or dim <= 1:
        return data.reshape(-1)
    return data.reshape(-1, dim)


def read_binary(filename, dim=1, dtype="d"):
    format_char, byte_size = sptk2struct[dtype]

    data = []

    with open(filename, "rb") as f:
        while True:
            buf = f.read(byte_size * dim)
            if not buf:
                break
            try:
                tmp = struct.unpack(format_char * dim, buf)
                data.append(tmp)
            except struct.error:
                break

    return asarray(data, dim=dim, dtype=dtype)


def read_stdin(dim=1, dtype="d"):
    format_char, byte_size = sptk2struct[dtype]

    source = sys.stdin.buffer
    data = []

    while True:
        buf = source.read(byte_size * dim)
        if not buf:
            break
        try:
            tmp = struct.unpack(format_char * dim, buf)
            data.append(tmp)
        except struct.error:
            break

    return asarray(data, dim=dim, dtype=dtype)


def print_error_message(name, message):
    print(f"{name}: {message}!", file=sys.stderr)
