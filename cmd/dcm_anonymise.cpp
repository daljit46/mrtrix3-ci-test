/*
    Copyright 2008 Brain Research Institute, Melbourne, Australia

    Written by J-Donald Tournier, 27/06/08.

    This file is part of MRtrix.

    MRtrix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MRtrix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "app.h"
#include "debug.h"
#include "file/path.h"
#include "file/dicom/element.h"
#include "file/dicom/quick_scan.h"
#include "file/dicom/definitions.h"

MRTRIX_APPLICATION

using namespace MR;
using namespace App;

void usage ()
{
  DESCRIPTION
  + "Anonymise DICOM file in-place. Note that this simply replace the existing "
    "values without modifying the DICOM structure in any way. Replacement text "
    "will be truncated if it is too long to fit inside the existing tag."

  + "WARNING: this command will modify existing data! It is recommended to run "
    "this command on a copy of the original data set to avoid loss of data."

  + "WARNING: there is no guarantee that this command will remove all identiable "
    "information. You will need to double-check the results independently if you "
    "need to ensure anonymity."

  + "By default, the following tags will be replaced:\n"
    " - any tag with Value Representation PN will be replaced with 'anonymous'\n"
    " - tag (0010,0030) PatientBirthDate will be replaced with an empty string";


  ARGUMENTS
  + Argument ("file", "the DICOM file to be scanned.").type_file ();

  OPTIONS
  + Option ("nodefault", "remove all default tags from replacement list.")

  + Option ("id", "replace all ID tags with string supplied.")
  +   Argument ("text")

  + Option ("tag", "replace specific tag.").allow_multiple()
  +   Argument ("group")
  +   Argument ("element")
  +   Argument ("newvalue");
}


class Tag {
  public:
    Tag (uint16_t group, uint16_t element, const std::string& newvalue) :
      group (group), element (element), newvalue (newvalue) { }
    uint16_t group, element;
    std::string newvalue;
};

inline std::string hex (uint16_t value)
{
  std::ostringstream hex;
  hex << std::hex << value;
  return hex.str();
}

inline uint16_t read_hex (const std::string& m)
{
  uint16_t value;
  std::istringstream hex (m);
  hex >> std::hex >> value;
  return value;
}

void run ()
{
  std::vector<Tag> tags;
  std::vector<uint16_t> VRs;

  Options opt = get_options ("nodefault");
  if (!opt.size()) {
    tags.push_back (Tag (0x0010U, 0x0030U, "")); // PatientBirthDate
    VRs.push_back (VR_PN);
  }


  opt = get_options ("tag");
  if (opt.size()) 
    for (size_t n = 0; n < opt.size(); ++n) 
      tags.push_back (Tag (read_hex (opt[n][0]), read_hex (opt[n][1]), opt[n][2]));

  opt = get_options ("id");
  if (opt.size()) {
    std::string newid = opt[0][0];
    tags.push_back (Tag (0x0010U, 0x0020U, newid)); // PatientID
    tags.push_back (Tag (0x0010U, 0x1000U, newid)); // OtherPatientIDs
  }

  for (size_t n = 0; n < VRs.size(); ++n) {
    union __VR { uint16_t i; char c[2]; } VR;
    VR.i = VRs[n];
    INFO (std::string ("clearing entries with VR \"") + VR.c[1] + VR.c[0] + "\"");
  }
  for (size_t n = 0; n < tags.size(); ++n) 
    INFO ("replacing tag (" + hex(tags[n].group) + "," + hex(tags[n].element) + ") with value \"" + tags[n].newvalue + "\"");

  File::Dicom::Element item;
  item.set (argument[0], true, true);
  while (item.read()) {
    for (size_t n = 0; n < VRs.size(); ++n) {
      if (item.VR == VRs[n]) {
        memset (item.data, 32, item.size);
        memcpy (item.data, "anonymous", std::min<int> (item.size, 9));
      }
    }
    for (size_t n = 0; n < tags.size(); ++n) {
      if (item.is (tags[n].group, tags[n].element)) {
        memset (item.data, 32, item.size);
        memcpy (item.data, tags[n].newvalue.c_str(), std::min<int> (item.size, tags[n].newvalue.size()));
      }
    }
  }
}

