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
    parser = argparse.ArgumentParser(description="draw graphs")
    parser.add_argument(
        metavar="infile",
        dest="in_file",
        default=None,
        nargs="?",
        type=str,
        help="data sequence (double)",
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
        "-t",
        dest="transpose",
        action="store_true",
        help="transpose axis",
    )
    parser.add_argument(
        "-b",
        dest="bar_graph_mode",
        action="store_true",
        help="set as bar graph mode",
    )
    parser.add_argument(
        "-n",
        metavar="n",
        dest="n_samples",
        default=None,
        type=int,
        help="number of samples per graph",
    )
    parser.add_argument(
        "-z",
        metavar="z",
        dest="bias",
        default=0.0,
        type=float,
        help="distance between graphs in the y-axis",
    )
    parser.add_argument(
        "-x",
        metavar=("XMIN", "XMAX"),
        dest="xlim",
        default=(None, None),
        nargs=2,
        type=float,
        help="x-axis limits",
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
        "-xname",
        metavar="XNAME",
        dest="xname",
        default=None,
        type=str,
        help="x-axis title",
    )
    parser.add_argument(
        "-yname",
        metavar="YNAME",
        dest="yname",
        default=None,
        type=str,
        help="y-axis title",
    )
    parser.add_argument(
        "-names",
        metavar="NAMES",
        dest="names",
        default=None,
        type=str,
        help="comma-separated graph names",
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
        default=1,
        type=int,
        help="line width",
    )
    parser.add_argument(
        "-ms",
        dest="marker_symbol",
        choices=utils.marker_symbols.keys(),
        default=0,
        type=int,
        help="marker symbol",
    )
    parser.add_argument(
        "-mc",
        metavar="mc",
        dest="marker_color",
        default=None,
        type=str,
        help="marker color",
    )
    parser.add_argument(
        "-mw",
        metavar="mw",
        dest="marker_size",
        default=None,
        type=int,
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
        default=1,
        type=int,
        help="marker line width",
    )
    return parser.parse_args()


##
# @a fdrw [ @e option ] [ @e infile ] @e outfile
#
# - @b -F @e float
#   - scale of figure
# - @b -W @e float
#   - width of figure in pixels
# - @b -H @e float
#   - height of figure in pixels
# - @b -g
#   - draw grid
# - @b -t
#   - transpose axis
# - @b -b
#   - set as bar graph mode
# - @b -n @e int
#   - number of samples per graph
# - @b -z @e float
#   - distance between graphs in the y-axis (requires -n option)
# - @b -x @e float @e float
#   - x-axis limits
# - @b -y @e float @e float
#   - y-axis limits
# - @b -xname @e str
#   - x-axis title
# - @b -yname @e str
#   - y-axis title
# - @b -names @e str
#   - comma-separated graph names
# - @b -ls @e str
#   - line style (solid, dash, dot, or dashdot)
# - @b -lc @e str
#   - line color
# - @b -lw @e str
#   - line width
# - @b -ms @e int
#   - marker symbol
# - @b -mc @e str
#   - marker color
# - @b -mw @e str
#   - marker size
# - @b -mlc @e str
#   - marker line color
# - @b -mlw @e str
#   - marker line width
# - @b infile @e str
#   - double-type data sequence
# - @b outfile @e str
#   - figure
#
# The below example draws the impulse response of a digital filter on @c out.png
# @code{.sh}
#   impulse -l 256 | dfs -a 1 0.8 0.5 | fdrw out.png
# @endcode
#
# The below example draws two sinusoidal lines from an input sequence.
# @code{.sh}
#   sin -l 256 -p 50 | fdrw -n 128 -names 1st,2nd out.png
# @endcode
def main():
    args = get_arguments()

    if args.in_file is None:
        if not utils.is_pipe():
            utils.print_error_message("fdrw", "Input file is not given")
            sys.exit(1)
        data = utils.read_stdin()
    else:
        if utils.is_pipe():
            utils.print_error_message("fdrw", "Too many input files")
            sys.exit(1)
        if not os.path.exists(args.in_file):
            utils.print_error_message("fdrw", f"Cannot open {args.in_file}")
            sys.exit(1)
        data = utils.read_binary(args.in_file)

    if args.marker_symbol == 0:
        mode = "lines"
    else:
        mode = "lines+markers"

    if args.names is None:
        names = None
    else:
        names = args.names.split(",")

    if args.n_samples is None or args.n_samples <= 0:
        step = len(data)
    else:
        step = args.n_samples

    fig = go.Figure()
    for i, j in enumerate(range(0, len(data), step)):
        x = np.arange(step)
        y = data[j : j + step] + (args.bias * i)

        if args.bar_graph_mode:
            fig.add_trace(
                go.Bar(
                    x=y if args.transpose else x,
                    y=x if args.transpose else y,
                    name=None if names is None else names[i],
                    orientation="h" if args.transpose else "v",
                    marker=dict(
                        color=args.marker_color,
                        line_color=args.marker_line_color,
                        line_width=args.marker_line_width,
                    ),
                )
            )
        else:
            fig.add_trace(
                go.Scatter(
                    x=y if args.transpose else x,
                    y=x if args.transpose else y,
                    mode=mode,
                    name=None if names is None else names[i],
                    line=dict(
                        color=args.line_color,
                        width=args.line_width,
                        dash=args.line_style,
                    ),
                    marker=dict(
                        symbol=utils.marker_symbols[args.marker_symbol],
                        color=args.marker_color,
                        size=args.marker_size,
                        line_color=args.marker_line_color,
                        line_width=args.marker_line_width,
                    ),
                )
            )
    fig.update_layout(
        xaxis=dict(
            title_text=args.yname if args.transpose else args.xname,
            range=args.ylim if args.transpose else args.xlim,
            showgrid=args.grid,
        ),
        yaxis=dict(
            title_text=args.xname if args.transpose else args.yname,
            range=args.xlim if args.transpose else args.ylim,
            showgrid=args.grid,
        ),
    )
    fig.write_image(
        args.out_file, width=args.width, height=args.height, scale=args.factor
    )


if __name__ == "__main__":
    main()
