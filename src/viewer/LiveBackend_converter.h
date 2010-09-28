#ifndef DSTORM_VIEWER_LIVEBACKEND_CONVERTER_H
#define DSTORM_VIEWER_LIVEBACKEND_CONVERTER_H

#include <dStorm/outputs/BinnedLocalizations.h>
#include "ColourDisplay.h"
#include "ImageDiscretizer_converter.h"
#include "Display.h"
#include "TerminalBackend.h"
#include "LiveBackend.h"
#include "Status_decl.h"

namespace dStorm {
namespace viewer {

template <int Hueing>
LiveBackend<Hueing>::LiveBackend(const TerminalBackend<Hueing>& other, Config &c, Status& s)
: config(c), status(s), 
  image( other.image ),
  colorizer( other.colorizer ),
  discretization( other.discretization,
                  image(), colorizer ),
  cache( 4096, other.cache.getSize().size ),
  cia( discretization, c, *this, colorizer, other.get_result() )
{
    image.setListener(&discretization);
    discretization.setListener(&cache);
    cache.setListener(&cia);

    cia.show_window();
}

template <int Hueing>
std::auto_ptr<Backend>
TerminalBackend<Hueing>::adapt( std::auto_ptr<Backend> self, Config& c, Status& s ) {
    assert( self.get() == this );

    if ( c.showOutput() ) {
        std::auto_ptr<Backend> fresh( new LiveBackend<Hueing>(*this, c, s) );
        std::swap( fresh, self );
        /* Self is now destructed! Take care not to modify or access this. */
    }

    return self;
}

}
}

#endif
