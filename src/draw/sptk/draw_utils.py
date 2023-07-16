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

import argparse
import struct
import sys
import warnings

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


def print_warn_message(name, message):
    warnings.warn(f"{name}: {message}", RuntimeWarning)


def print_error_message(name, message):
    print(f"{name}: {message}!", file=sys.stderr)


def get_default_parser(description, input_name=None, allow_dtype=True):
    class MarginAction(argparse.Action):
        def __init__(self, option_strings, dest, nargs=None, **kwargs):
            if nargs is not None:
                raise ValueError("nargs not allowed")
            super().__init__(option_strings, dest, **kwargs)

        def __call__(self, parser, namespace, values, option_string=None):
            if values is not None:
                v = [int(x) for x in values.split(",")]
                if len(v) == 1:
                    values = dict(l=v[0], r=v[0], t=v[0], b=v[0])
                elif len(v) == 2:
                    values = dict(l=v[0], r=v[0], t=v[1], b=v[1])
                elif len(v) == 4:
                    values = dict(l=v[0], r=v[1], t=v[2], b=v[3])
                else:
                    raise ValueError("unexpected margin")
            setattr(namespace, self.dest, values)

    parser = argparse.ArgumentParser(description=description, prefix_chars="-+")
    if input_name is not None:
        parser.add_argument(
            metavar="infile",
            dest="in_file",
            default=None,
            nargs="?",
            type=str,
            help=input_name,
        )
    parser.add_argument(
        metavar="outfile",
        dest="out_file",
        type=str,
        help="figure",
    )
    parser.add_argument(
        "-F",
        metavar="F",
        dest="factor",
        default=1,
        type=float,
        help="scale of figure",
    )
    parser.add_argument(
        "-W",
        metavar="W",
        dest="width",
        default=None,
        type=int,
        help="width of figure [px]",
    )
    parser.add_argument(
        "-H",
        metavar="H",
        dest="height",
        default=None,
        type=int,
        help="height of figure [px]",
    )
    parser.add_argument(
        "-M",
        metavar="M",
        dest="margin",
        default=None,
        action=MarginAction,
        type=str,
        help="comma-separated margin (l,r,t,b) [px]",
    )
    parser.add_argument(
        "-ff",
        metavar="ff",
        dest="font_family",
        default=None,
        type=str,
        help="font family",
    )
    parser.add_argument(
        "-fs",
        metavar="fs",
        dest="font_size",
        default=None,
        type=int,
        help="font size",
    )
    if allow_dtype:
        parser.add_argument(
            "+c",
            dest="dtype",
            action="store_const",
            const="c",
            help="(data type) char, 1byte",
        )
        parser.add_argument(
            "+C",
            dest="dtype",
            action="store_const",
            const="C",
            help="(data type) unsigned char, 1byte",
        )
        parser.add_argument(
            "+s",
            dest="dtype",
            action="store_const",
            const="s",
            help="(data type) short, 2byte",
        )
        parser.add_argument(
            "+S",
            dest="dtype",
            action="store_const",
            const="S",
            help="(data type) unsigned short, 2byte",
        )
        parser.add_argument(
            "+i",
            dest="dtype",
            action="store_const",
            const="i",
            help="(data type) int, 4byte",
        )
        parser.add_argument(
            "+I",
            dest="dtype",
            action="store_const",
            const="I",
            help="(data type) unsigned int, 4byte",
        )
        parser.add_argument(
            "+l",
            dest="dtype",
            action="store_const",
            const="l",
            help="(data type) long, 8byte",
        )
        parser.add_argument(
            "+L",
            dest="dtype",
            action="store_const",
            const="L",
            help="(data type) unsigned long, 8byte",
        )
        parser.add_argument(
            "+f",
            dest="dtype",
            action="store_const",
            const="f",
            help="(data type) float, 4byte",
        )
        parser.add_argument(
            "+d",
            dest="dtype",
            action="store_const",
            const="d",
            help="(data type) double, 8byte",
        )
    parser.set_defaults(dtype="d")
    return parser
