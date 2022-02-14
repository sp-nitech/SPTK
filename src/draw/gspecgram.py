#!/usr/bin/env python
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
import os
import sys

import numpy as np
import plotly.graph_objs as go
from plotly.subplots import make_subplots
from scipy import signal

import sptk.draw_utils as utils


def get_arguments():
    parser = argparse.ArgumentParser(description="draw a spectrogram")
    parser.add_argument(
        metavar="infile",
        dest="in_file",
        default=None,
        nargs="?",
        type=str,
        help="waveform (double)",
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
        default=1.0,
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
        "-s",
        metavar="s",
        dest="start_point",
        default=0,
        type=int,
        help="start point",
    )
    parser.add_argument(
        "-e",
        metavar="e",
        dest="end_point",
        default=None,
        type=int,
        help="end point",
    )
    parser.add_argument(
        "-n",
        metavar="n",
        dest="num_samples",
        default=None,
        type=int,
        help="number of samples per screen",
    )
    parser.add_argument(
        "-i",
        metavar="i",
        dest="num_screens",
        default=1,
        type=int,
        help="number of screens",
    )
    parser.add_argument(
        "-y",
        metavar="y",
        dest="sr",
        default=16,
        type=float,
        help="sampling rate [kHz]",
    )
    parser.add_argument(
        "-w",
        metavar="w",
        dest="window_type",
        default="blackman",
        type=str,
        help="window type",
    )
    parser.add_argument(
        "-l",
        metavar="l",
        dest="window_length",
        default=512,
        type=int,
        help="window length",
    )
    parser.add_argument(
        "-c",
        metavar="c",
        dest="color_scale",
        default="Viridis",
        type=str,
        help="color scale",
    )
    parser.add_argument(
        "-p",
        metavar="p",
        dest="power",
        default=1,
        type=float,
        help="power parameter",
    )
    return parser.parse_args()


##
# @a gspecgram [ @e option ] [ @e infile ] @e outfile
#
# - @b -F @e float
#   - scale of figure
# - @b -W @e int
#   - width of figure in pixels
# - @b -H @e int
#   - height of figure in pixels
# - @b -s @e int
#   - start point
# - @b -e @e int
#   - end point
# - @b -n @e int
#   - number of samples per screen
# - @b -i @e int
#   - number of screens
# - @b -y @e int
#   - sampling rate in kHz
# - @b -w @e str
#   - window type
# - @b -l @e int
#   - window length
# - @b -c @e str
#   - color scale
# - @b -p @e float
#   - a parameter to control visibility
# - @b infile @e str
#   - double-type waveform
# - @b outfile @e str
#   - figure
#
# The below example draws an entire spectrogram.
# @code{.sh}
#   x2x +sd data.short | gspecgram out.png
# @endcode
def main():
    args = get_arguments()

    if args.in_file is None:
        data = utils.read_stdin()
    else:
        if not os.path.exists(args.in_file):
            utils.print_error_message("gspecgram", f"Cannot open {args.in_file}")
            sys.exit(1)
        data = utils.read_binary(args.in_file)

    y = data[args.start_point : None if args.end_point is None else args.end_point + 1]

    if args.num_samples is None:
        n = len(y) // args.num_screens
    else:
        n = args.num_samples

    fig = make_subplots(rows=args.num_screens, cols=1)
    s = 0
    bias = 0
    for i in range(args.num_screens):
        last = i == args.num_screens - 1
        if args.num_samples is None and last:
            e = len(y)
        else:
            e = s + n

        freq, time, spec = signal.spectrogram(
            y[s:e], fs=args.sr, window=args.window_type, nperseg=args.window_length
        )
        spec = 10.0 * np.log10(spec)
        if args.power != 1.0:
            spec = (spec - np.min(spec)) ** args.power

        fig.add_trace(
            go.Heatmap(
                x=(time + bias) * 0.001,
                y=freq,
                z=spec,
                colorscale=args.color_scale,
            ),
            row=i + 1,
            col=1,
        )
        fig.update_xaxes(
            title_text="Time [sec]" if last else "",
            row=i + 1,
            col=1,
        ),
        fig.update_yaxes(
            title_text="Frequency [kHz]",
            row=i + 1,
            col=1,
        )
        s = e
        bias = time[-1]

    # Hide reference color bar.
    fig.update_traces(showscale=False)

    fig.write_image(
        args.out_file,
        width=args.width,
        height=args.height,
        scale=args.factor,
    )


if __name__ == "__main__":
    main()
