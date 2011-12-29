#ifndef DSTORM_VIEWER_TERMINALCACHE_H
#define DSTORM_VIEWER_TERMINALCACHE_H

#include <dStorm/display/DataSource.h>
#include "ImageDiscretizer.h"

namespace dStorm {
namespace viewer {

template <typename MyColorizer>
class TerminalCache 
: public DummyDiscretizationListener
{
    typedef display::Image Im;

    dStorm::display::ResizeChange size;

  public:
    typedef MyColorizer Colorizer;

    TerminalCache();

    const dStorm::display::ResizeChange& getSize() const 
        { return size; }
    void setSize(const input::Traits< Image<int,2> >&);
    void setSize(const dStorm::display::ResizeChange& size);

    std::auto_ptr<dStorm::display::Change> 
    get_result(const Colorizer& colorizer) const;
};

}
}
#endif
