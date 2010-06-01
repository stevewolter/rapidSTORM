#ifndef DSTORM_VIEWER_TERMINALCACHE_H
#define DSTORM_VIEWER_TERMINALCACHE_H

#include <dStorm/helpers/DisplayDataSource.h>
#include "ImageDiscretizer.h"

namespace dStorm {
namespace viewer {

template <typename MyColorizer>
class TerminalCache 
: public DummyDiscretizationListener
{
    typedef dStorm::Image<dStorm::Pixel,2> Im;

    dStorm::Display::ResizeChange size;

  public:
    TerminalCache() {}
    TerminalCache(dStorm::Display::ResizeChange size);

    typedef MyColorizer Colorizer;

    void setSize(const input::Traits< Image<int,2> >&);

    std::auto_ptr<dStorm::Display::Change> get_result();
};

}
}
#endif
