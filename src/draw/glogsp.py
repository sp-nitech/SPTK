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

import sptk.draw_utils as utils


def get_arguments():
    parser = argparse.ArgumentParser(description="draw a log spectrum graph")
    parser.add_argument(
        metavar="infile",
        dest="in_file",
        default=None,
        nargs="?",
        type=str,
        help="log spectrum (double)",
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
        "-g",
        dest="grid",
        action="store_true",
        help="draw grid",
    )
    parser.add_argument(
        "-s",
        metavar="s",
        dest="frame_number",
        default=0,
        type=int,
        help="frame number",
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
        "-x",
        metavar="x",
        dest="sr",
        default=1,
        type=float,
        help="sampling rate [kHz]",
    )
    parser.add_argument(
        "-y",
        metavar=("YMIN", "YMAX"),
        dest="ylim",
        default=(None, None),
        nargs=2,
        type=float,
        help="y-axis limits",
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
        default=None,
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
# @a glogsp [ @e option ] [ @e infile ] @e outfile
#
# - @b -F @e float
#   - scale of figure
# - @b -W @e int
#   - width of figure in pixels
# - @b -H @e int
#   - height of figure in pixels
# - @b -g
#   - draw grid
# - @b -s @e int
#   - frame number
# - @b -l @e int
#   - frame length
# - @b -x @e float
#   - sampling rate in kHz
# - @b -y @e float @e float
#   - y-axis limits
# - @b -ls @e str
#   - line style (solid, dash, dot, or dashdot)
# - @b -lc @e str
#   - line color
# - @b -lw @e float
#   - line width
# - @b infile @e str
#   - double-type log spectrum
# - @b outfile @e str
#   - figure
#
# The below example draws 100th log spectrum on @c out.jpg.
# @code{.sh}
#   x2x +sd data.short | frame -p 80 -l 400 | window -l 400 -L 512 | spec -l 512 |
#     glogsp -l 512 -s 100 out.jpg
# @endcode
def main():
    args = get_arguments()

    dim = args.frame_length // 2 + 1

    if args.in_file is None:
        data = utils.read_stdin(dim=dim)
    else:
        if not os.path.exists(args.in_file):
            utils.print_error_message("glogsp", f"Cannot open {args.in_file}")
            sys.exit(1)
        data = utils.read_binary(args.in_file, dim=dim)

    x = np.linspace(0, 1, dim)
    if 0 < args.sr:
        x *= args.sr * 0.5  # Multiply Nyquist frequency.
    y = data[args.frame_number]

    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            x=x,
            y=y,
            line=dict(
                color=args.line_color,
                width=args.line_width,
                dash=args.line_style,
            ),
        )
    )
    if args.sr <= 0:
        fig.update_xaxes(
            title_text="Frequency [rad]",
            tickmode="array",
            tickvals=[0, 0.5, 1],
            ticktext=["$0$", r"$\pi/2$", r"$\pi$"],
        )
    elif args.sr == 1:
        fig.update_xaxes(title_text="Normalized frequency [cyc]")
    else:
        fig.update_xaxes(title_text="Frequency [kHz]")
    fig.update_layout(
        xaxis=dict(
            range=(x[0], x[-1]),
            showgrid=args.grid,
        ),
        yaxis=dict(
            title_text="Log amplitude [dB]",
            range=args.ylim,
            showgrid=args.grid,
        ),
    )
    fig.write_image(
        args.out_file, width=args.width, height=args.height, scale=args.factor
    )


if __name__ == "__main__":
    main()
