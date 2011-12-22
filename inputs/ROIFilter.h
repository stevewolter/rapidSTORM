#ifndef DSTORM_INPUT_ROIFILTER_H
#define DSTORM_INPUT_ROIFILTER_H

#include <dStorm/input/chain/Link_decl.h>
#include <memory>

namespace dStorm {
namespace ROIFilter {

std::auto_ptr<input::chain::Link> make_link();

}
}

#endif
