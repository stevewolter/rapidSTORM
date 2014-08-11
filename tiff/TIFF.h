#ifndef CImgBuffer_TIFF_H
#define CImgBuffer_TIFF_H

#include <memory>

#include "engine/Image.h"
#include "image/fwd.h"
#include "image/MetaInfo.h"
#include "input/Link.h"

namespace boost { namespace unit_test { class test_suite; } }
namespace dStorm {
namespace tiff {

extern const std::string test_file_name;
std::unique_ptr< input::Link<engine::ImageStack> > make_input();
boost::unit_test::test_suite* register_unit_tests();

}

}

#endif
