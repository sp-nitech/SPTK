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

import sptk.draw_utils as utils


def get_arguments():
    parser = utils.get_default_parser(
        "draw a running log spectrum", input_name="log spectrum", allow_dtype=False
    )
    parser.add_argument(
        "-g",
        dest="grid",
        action="store_true",
        help="draw grid",
    )
    parser.add_argument(
        "-t",
        dest="transpose",
        action="store_true",
        help="transpose axis",
    )
    parser.add_argument(
        "-s",
        metavar="s",
        dest="start_frame_number",
        default=0,
        type=int,
        help="start frame number",
    )
    parser.add_argument(
        "-e",
        metavar="e",
        dest="end_frame_number",
        default=None,
        type=int,
        help="end frame number",
    )
    parser.add_argument(
        "-l",
        metavar="l",
        dest="frame_length",
        default=256,
        type=int,
        help="frame length (FFT length)",
    )
    parser.add_argument(
        "-z",
        metavar="z",
        dest="bias",
        default=20,
        type=float,
        help="distance between spectra",
    )
    parser.add_argument(
        "-x",
        metavar="x",
        dest="sr",
        default=1,
        type=float,
        help="sampling rate [kHz]",
    )
    parser.add_argument(
        "-y",
        metavar="y",
        dest="margin_factor",
        default=2,
        type=float,
        help="margin factor",
    )
    parser.add_argument(
        "-ls",
        dest="line_style",
        choices=utils.line_styles,
        default="solid",
        type=str,
        help="line style",
    )
    parser.add_argument(
        "-lc",
        metavar="lc",
        dest="line_color",
        default="#636EFA",
        type=str,
        help="line color",
    )
    parser.add_argument(
        "-lw",
        metavar="lw",
        dest="line_width",
        default=None,
        type=float,
        help="line width",
    )
    return parser.parse_args()


##
# @a grlogsp [ @e option ] [ @e infile ] @e outfile
#
# - @b -F @e float
#   - scale of figure
# - @b -W @e int
#   - width of figure in pixels
# - @b -H @e int
#   - height of figure in pixels
# - @b -M @e int or str
#   - margin around image in pixels
# - @b -g
#   - draw grid
# - @b -t
#   - transpose axis
# - @b -s @e int
#   - start frame number
# - @b -e @e int
#   - end frame number
# - @b -l @e int
#   - frame length
# - @b -z @e float
#   - distance between spectra in decibels
# - @b -x @e float
#   - sampling rate in kHz
# - @b -y @e float
#   - margin factor
# - @b -ls @e str
#   - line style (solid, dash, dot, or dashdot)
# - @b -lc @e str
#   - line color
# - @b -lw @e float
#   - line width
# - @b -ff @e str
#   - font family
# - @b -fs @e int
#   - font size
# - @b infile @e str
#   - double-type log spectrum
# - @b outfile @e str
#   - figure
#
# The below example draws a running log spectrum on @c out.eps.
# @code{.sh}
#   x2x +sd data.short | frame -p 80 -l 400 | window -l 400 -L 512 | spec -l 512 |
#     grlogsp -l 512 -H 6000 out.eps
# @endcode
def main():
    args = get_arguments()

    dim = args.frame_length // 2 + 1

    if args.in_file is None:
        data = utils.read_stdin(dim=dim)
    else:
        if not os.path.exists(args.in_file):
            utils.print_error_message("grlogsp", f"Cannot open {args.in_file}")
            sys.exit(1)
        data = utils.read_binary(args.in_file, dim=dim)

    x = np.linspace(0, 1, dim)
    if 0 < args.sr:
        x *= args.sr * 0.5  # Multiply Nyquist frequency.
    if args.transpose:
        x = np.flip(x)
    ys = data[
        args.start_frame_number : (
            None if args.end_frame_number is None else args.end_frame_number + 1
        )
    ]

    fig = go.Figure()
    y_min = np.inf
    y_max = -np.inf
    for i in range(len(ys)):
        y = ys[i] + (args.bias * i)
        y_min = np.minimum(y_min, np.min(y))
        y_max = np.maximum(y_max, np.max(y))

        fig.add_trace(
            go.Scatter(
                x=np.flip(y) if args.transpose else x,
                y=x if args.transpose else y,
                line=dict(
                    color=args.line_color,
                    width=args.line_width,
                    dash=args.line_style,
                ),
            )
        )
    xaxis = dict(
        range=(x[0], x[-1]),
        showgrid=args.grid,
    )
    yaxis = dict(
        range=(
            y_min - args.margin_factor * args.bias,
            y_max + args.margin_factor * args.bias,
        ),
        showticklabels=False,
        showgrid=False,
        zeroline=False,
    )
    if args.sr <= 0:
        xaxis["title_text"] = "Normalized frequency [rad]"
        xaxis["tickmode"] = "array"
        xaxis["tickvals"] = [0, 0.5, 1]
        xaxis["ticktext"] = ["$0$", r"$\pi/2$", r"$\pi$"]
    elif args.sr == 1:
        xaxis["title_text"] = "Normalized frequency [cyc]"
    else:
        xaxis["title_text"] = "Frequency [kHz]"
    fig.update_layout(
        xaxis=yaxis if args.transpose else xaxis,
        yaxis=xaxis if args.transpose else yaxis,
        font=dict(
            family=args.font_family,
            size=args.font_size,
        ),
        margin=args.margin,
        showlegend=False,
    )
    fig.write_image(
        args.out_file, width=args.width, height=args.height, scale=args.factor
    )


if __name__ == "__main__":
    main()
