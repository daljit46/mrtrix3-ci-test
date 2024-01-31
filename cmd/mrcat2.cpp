/* Copyright (c) 2008-2024 the MRtrix3 contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Covered Software is provided under this License on an "as is"
 * basis, without warranty of any kind, either expressed, implied, or
 * statutory, including, without limitation, warranties that the
 * Covered Software is free of defects, merchantable, fit for a
 * particular purpose or non-infringing.
 * See the Mozilla Public License v. 2.0 for more details.
 *
 * For more details, see http://www.mrtrix.org/.
 */

#include "algo/loop.h"
#include "command.h"
#include "dwi/gradient.h"
#include "image.h"
#include "phase_encoding.h"
#include "progressbar.h"

using namespace MR;
using namespace App;

void usage() {
  AUTHOR = "J-Donald Tournier (jdtournier@gmail.com) and Robert E. Smith (robert.smith@florey.edu.au)";

  SYNOPSIS = "Concatenate several images into one";

  ARGUMENTS
  +Argument("image1", "the first input image.").type_image_in()

      + Argument("image2", "additional input image(s).").type_image_in().allow_multiple()

      + Argument("output", "the output image.").type_image_out();

  EXAMPLES
  +Example("Concatenate individual 3D volumes into a single 4D image series",
           "mrcat volume*.mif series.mif",
           "The wildcard characters will find all images in the current working directory with names that "
           "begin with \"volume\" and end with \".mif\"; the mrcat command will receive these as a list of "
           "input file names, from which it will produce a 4D image where the input volumes have been "
           "concatenated along axis 3 (the fourth axis; the spatial axes are 0, 1 & 2).");

  OPTIONS
  +Option("axis",
          "specify axis along which concatenation should be performed. By default, "
          "the program will use the last non-singleton, non-spatial axis of any of "
          "the input images - in other words axis 3 or whichever axis (greater than 3) "
          "of the input images has size greater than one.") +
      Argument("axis").type_integer(0)

      + DataType::options();
}

template <typename value_type> void write(std::vector<Header> &in, const size_t axis, Header &header_out) {
  auto image_out = Image<value_type>::create(header_out.name(), header_out);
  size_t axis_offset = 0;

  for (size_t i = 0; i != in.size(); i++) {
    auto image_in = in[i].get_image<value_type>();

    auto copy_func = [&axis, &axis_offset](decltype(image_in) &in, decltype(image_out) &out) {
      out.index(axis) = axis < in.ndim() ? in.index(axis) + axis_offset : axis_offset;
      out.value() = in.value();
    };

    ThreadedLoop(
        "concatenating \"" + image_in.name() + "\"", image_in, 0, std::min<size_t>(image_in.ndim(), image_out.ndim()))
        .run(copy_func, image_in, image_out);
    if (axis < image_in.ndim())
      axis_offset += image_in.size(axis);
    else {
      ++axis_offset;
      image_out.index(axis) = axis_offset;
    }
  }
}

void run() {
  for(int i = 0; i < 10; i++){
    std::cout << i << std::endl;
  }
}
