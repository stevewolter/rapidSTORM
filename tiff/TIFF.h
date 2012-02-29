#ifndef CImgBuffer_TIFF_H
#define CImgBuffer_TIFF_H

#include <dStorm/engine/InputTraits.h>
#include <dStorm/image/fwd.h>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/input/FileInput.h>
#include <dStorm/input/Source.h>
#include <memory>

struct TestState;

namespace dStorm {
namespace tiff {

extern const std::string test_file_name;
std::auto_ptr< input::Link > make_input();
void unit_test( TestState& );

}

}

#endif
