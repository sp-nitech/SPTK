#!/usr/bin/env python3
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

import os
import sys

import numpy as np
import plotly.graph_objs as go
from plotly.subplots import make_subplots
from scipy import signal

import sptk.draw_utils as utils


def get_arguments():
    parser = utils.get_default_parser("draw spectrogram", input_name="waveform")
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
        "-t",
        dest="transpose",
        action="store_true",
        help="align screens horizontally instead of vertically (valid with -i)",
    )
    parser.add_argument(
        "-r",
        dest="reset",
        action="store_true",
        help="do not succeed time across screens (valid with -i)",
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
    parser.add_argument(
        "-a",
        metavar="a",
        dest="eps",
        default=1e-8,
        type=float,
        help="small value added to power spectrum",
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
# - @b -M @e int or str
#   - margin around image in pixels
# - @b -s @e int
#   - start point
# - @b -e @e int
#   - end point
# - @b -n @e int
#   - number of samples per screen
# - @b -i @e int
#   - number of screens
# - @b -t
#   - align figures horizontally instead of vertically
# - @b -r
#   - do not succeed time across screens
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
# - @b -a @e float
#   - small value added to power spectrum
# - @b -ff @e str
#   - font family
# - @b -fs @e int
#   - font size
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
        data = utils.read_stdin(dtype=args.dtype)
    else:
        if not os.path.exists(args.in_file):
            utils.print_error_message("gspecgram", f"Cannot open {args.in_file}")
            sys.exit(1)
        data = utils.read_binary(args.in_file, dtype=args.dtype)

    y = data[args.start_point : None if args.end_point is None else args.end_point + 1]

    if args.num_samples is None:
        n = len(y) // args.num_screens
    else:
        n = args.num_samples

    fig = make_subplots(
        rows=1 if args.transpose else args.num_screens,
        cols=args.num_screens if args.transpose else 1,
    )
    s = 0
    bias = 0
    for i in range(args.num_screens):
        first = i == 0
        last = i == args.num_screens - 1
        row_col = {
            "row": 1 if args.transpose else i + 1,
            "col": i + 1 if args.transpose else 1,
        }
        if args.num_samples is None and last:
            e = len(y)
        else:
            e = s + n

        freq, time, spec = signal.spectrogram(
            y[s:e], fs=args.sr, window=args.window_type, nperseg=args.window_length
        )
        spec = 10.0 * np.log10(spec + args.eps)
        if args.power != 1.0:
            spec = (spec - np.min(spec)) ** args.power

        fig.add_trace(
            go.Heatmap(
                x=(time + bias) * 0.001,
                y=freq,
                z=spec,
                colorscale=args.color_scale,
            ),
            **row_col,
        )
        fig.update_xaxes(
            title_text="Time [sec]" if last or args.transpose else "",
            **row_col,
        )
        fig.update_yaxes(
            title_text="Frequency [kHz]" if first or not args.transpose else "",
            **row_col,
        )
        s = e
        if not args.reset:
            bias = time[-1]

    fig.update_layout(
        font=dict(
            family=args.font_family,
            size=args.font_size,
        ),
        margin=args.margin,
    )

    # Hide reference color bar.
    fig.update_traces(showscale=False)

    fig.write_image(
        args.out_file, width=args.width, height=args.height, scale=args.factor
    )


if __name__ == "__main__":
    main()
