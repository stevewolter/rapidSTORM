#ifndef DSTORM_INPUT_ROIFILTER_DECL_H
#define DSTORM_INPUT_ROIFILTER_DECL_H

#include <memory>
#include <dStorm/input/chain/Filter_decl.h>

namespace dStorm {
namespace ROIFilter {

class Config;
template <typename Type> class Source;
template <typename Type> class TypedFilter;
class FullFilter;

std::auto_ptr<input::chain::Filter> makeFilter();

}
}

#endif
