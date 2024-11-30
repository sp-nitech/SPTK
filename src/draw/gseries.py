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

import sptk.draw_utils as utils


def get_arguments():
    parser = utils.get_default_parser(
        "draw a dicrete series", input_name="discrete series"
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
        "-x",
        metavar="x",
        dest="sr",
        default=None,
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
        default=0.02,
        type=float,
        help="line width",
    )
    parser.add_argument(
        "-ms",
        dest="marker_symbol",
        choices=utils.marker_symbols.keys(),
        default=1,
        type=int,
        help="marker symbol",
    )
    parser.add_argument(
        "-mc",
        metavar="mc",
        dest="marker_color",
        default="#636EFA",
        type=str,
        help="marker color",
    )
    parser.add_argument(
        "-mw",
        metavar="mw",
        dest="marker_size",
        default=None,
        type=float,
        help="marker size",
    )
    parser.add_argument(
        "-mlc",
        metavar="mlc",
        dest="marker_line_color",
        default="midnightblue",
        type=str,
        help="marker line color",
    )
    parser.add_argument(
        "-mlw",
        metavar="mlw",
        dest="marker_line_width",
        default=None,
        type=float,
        help="marker line width",
    )
    return parser.parse_args()


##
# @a gseries [ @e option ] [ @e infile ] @e outfile
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
# - @b -x @e float
#   - sampling rate in kHz
# - @b -y @e float @e float
#   - y-axis limits
# - @b -lc @e str
#   - line color
# - @b -lw @e float
#   - line width
# - @b -ms @e int
#   - marker symbol
# - @b -mc @e str
#   - marker color
# - @b -mw @e float
#   - marker size
# - @b -mlc @e str
#   - marker line color
# - @b -mlw @e float
#   - marker line width
# - @b -ff @e str
#   - font family
# - @b -fs @e int
#   - font size
# - @b infile @e str
#   - double-type discrete series
# - @b outfile @e str
#   - figure
#
# The below example draws the impulse response of a digital filter on @c out.jpeg.
# @code{.sh}
#   impulse -l 256 | dfs -a 1 0.8 0.5 | gseries out.jpeg
# @endcode
def main():
    args = get_arguments()

    if args.in_file is None:
        data = utils.read_stdin(dtype=args.dtype)
    else:
        if not os.path.exists(args.in_file):
            utils.print_error_message("gseries", f"Cannot open {args.in_file}")
            sys.exit(1)
        data = utils.read_binary(args.in_file, dtype=args.dtype)

    y = data[args.start_point : None if args.end_point is None else args.end_point + 1]
    x = np.arange(len(y)) + args.start_point
    if args.sr is not None:
        x = x / (args.sr * 1000)

    if 100000 < len(y):
        utils.print_warn_message(
            "gseries", "Too many data points. This takes a long time."
        )

    if args.ylim[0] is None:
        ymax = np.amax(np.abs(y))
        ylim = (-ymax, ymax)
    else:
        ylim = args.ylim

    if args.num_samples is None:
        n = len(y) // args.num_screens
    else:
        n = args.num_samples

    fig = make_subplots(
        rows=1 if args.transpose else args.num_screens,
        cols=args.num_screens if args.transpose else 1,
    )
    s = 0
    for i in range(args.num_screens):
        last = i == args.num_screens - 1
        row_col = {
            "row": 1 if args.transpose else i + 1,
            "col": i + 1 if args.transpose else 1,
        }
        if args.num_samples is None and last:
            e = len(y)
        else:
            e = s + n
        fig.add_trace(
            go.Bar(
                x=x[: e - s] if args.reset else x[s:e],
                y=y[s:e],
                width=(
                    args.line_width
                    if args.sr is None
                    else args.line_width / (args.sr * 1000)
                ),
                marker=dict(
                    color=args.line_color,
                    line_width=0,
                ),
            ),
            **row_col,
        )
        fig.add_trace(
            go.Scatter(
                x=x[: e - s] if args.reset else x[s:e],
                y=y[s:e],
                mode="markers",
                marker=dict(
                    symbol=utils.marker_symbols[args.marker_symbol],
                    color=args.marker_color,
                    size=args.marker_size,
                    line_color=args.marker_line_color,
                    line_width=args.marker_line_width,
                ),
            ),
            **row_col,
        )
        xname = "Time [samples]" if args.sr is None else "Time [sec]"
        fig.update_xaxes(
            title_text=xname if last or args.transpose else "",
            showgrid=args.grid,
            **row_col,
        )
        fig.update_yaxes(
            range=ylim,
            showgrid=args.grid,
            **row_col,
        )
        s = e

    fig.update_layout(
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
