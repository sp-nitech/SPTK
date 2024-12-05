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
    parser = utils.get_default_parser("draw poles and zeros", allow_dtype=False)
    parser.add_argument(
        "-g",
        dest="grid",
        action="store_true",
        help="draw grid",
    )
    parser.add_argument(
        "-q",
        metavar="q",
        dest="input_format",
        default=0,
        type=int,
        help="input format (0: rectangular form, 1: polar form)",
    )
    parser.add_argument(
        "-z",
        metavar="z",
        dest="zero_file",
        default=None,
        type=str,
        help="roots of zeros (double)",
    )
    parser.add_argument(
        "-p",
        metavar="p",
        dest="pole_file",
        default=None,
        type=str,
        help="roots of poles (double)",
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
        default=10,
        type=float,
        help="marker size",
    )
    return parser.parse_args()


##
# @a gpolezero [ @e option ] @e outfile
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
# - @b -q @e int
#   - input format
# - @b -p @e str
#   - name of file containing poles
# - @b -z @e str
#   - name of file containing zeros
# - @b -x @e float @e float
#   - x-axis limits
# - @b -y @e float @e float
#   - y-axis limits
# - @b -mc @e str
#   - marker color
# - @b -mw @e float
#   - marker size
# - @b -ff @e str
#   - font family
# - @b -fs @e int
#   - font size
# - @b infile @e str
#   - double-type roots
# - @b outfile @e str
#   - figure
#
# The below example draws the poles and zeros of the transfer function:
# @f[
#   H(z) = \frac{2z^2 + 3z + 1}{z^2 + 0.8z + 0.5}.
# @f]
# @code{.sh}
#   echo 2 3 1 | x2x +ad | root_pol -m 2 > data.z
#   echo 1 0.8 0.5 | x2x +ad | root_pol -m 2 > data.p
#   gpolezero -p data.p -z data.z out.png
# @endcode
def main():
    args = get_arguments()

    if args.zero_file is None and args.pole_file is None:
        in_files = [None]
    else:
        in_files = [args.zero_file, args.pole_file]

    markers = (
        dict(
            symbol="circle-open",
            color=args.marker_color,
            size=args.marker_size,
            line_color=None,
            line_width=0,
        ),
        dict(
            symbol="x-open",
            color=args.marker_color,
            size=args.marker_size,
            line_color=None,
            line_width=0,
        ),
    )

    fig = go.Figure()

    # Draw a unit circle.
    fig.add_shape(
        type="circle",
        xref="x",
        yref="y",
        x0=-1,
        y0=-1,
        x1=1,
        y1=1,
        line_color="purple",
    )

    # Draw zeros and poles.
    for in_file, marker in zip(in_files, markers):
        if len(in_files) == 1:
            data = utils.read_stdin(dim=2)
        else:
            if in_file is None:
                continue
            if not os.path.exists(in_file):
                utils.print_error_message("gpolezero", f"Cannot open {in_file}")
                sys.exit(1)
            data = utils.read_binary(in_file, dim=2)

        if 0 == args.input_format:
            x = data[:, 0]
            y = data[:, 1]
        elif 1 == args.input_format:
            r = data[:, 0]
            theta = data[:, 1]
            x = r * np.cos(theta)
            y = r * np.sin(theta)
        else:
            utils.print_error_message("gpolezero", "Unexpected input format")
            sys.exit(1)

        fig.add_trace(
            go.Scatter(
                x=x,
                y=y,
                mode="markers",
                marker=marker,
            )
        )

    fig.update_layout(
        xaxis=dict(
            title_text="Real part",
            range=args.xlim,
            showgrid=args.grid,
            zeroline=True,
        ),
        yaxis=dict(
            title_text="Imaginary part",
            range=args.ylim,
            showgrid=args.grid,
            zeroline=True,
            scaleanchor="x",
            scaleratio=1,
        ),
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
