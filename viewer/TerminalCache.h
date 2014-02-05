#ifndef DSTORM_VIEWER_TERMINALCACHE_H
#define DSTORM_VIEWER_TERMINALCACHE_H

#include <dStorm/display/DataSource.h>
#include "viewer/ImageDiscretizer.h"

namespace dStorm {
namespace viewer {

class TerminalCache 
: public DummyDiscretizationListener
{
    display::ResizeChange size;

  public:
    TerminalCache();

    const display::ResizeChange& getSize() const 
        { return size; }
    void setSize(const image::MetaInfo<Im::Dim>&);
    void setSize(const dStorm::display::ResizeChange& size);
};

}
}
#endif
