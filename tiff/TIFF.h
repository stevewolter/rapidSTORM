#ifndef CImgBuffer_TIFF_H
#define CImgBuffer_TIFF_H

#include <dStorm/engine/InputTraits.h>
#include <dStorm/image/fwd.h>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/input/FileInput.h>
#include <dStorm/input/Source.h>
#include <memory>

namespace boost { namespace unit_test { class test_suite; } }
namespace dStorm {
namespace tiff {

extern const std::string test_file_name;
std::auto_ptr< input::Link > make_input();
boost::unit_test::test_suite* register_unit_tests();

}

}

#endif
