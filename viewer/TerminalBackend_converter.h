#ifndef DSTORM_VIEWER_TERMBACKEND_CONVERTER_H
#define DSTORM_VIEWER_TERMBACKEND_CONVERTER_H

#include <dStorm/outputs/BinnedLocalizations.h>
#include "ImageDiscretizer_converter.h"
#include "Display.h"
#include "LiveBackend.h"
#include "TerminalBackend.h"
#include "Config.h"
#include "Status.h"

namespace dStorm {
namespace viewer {

template <typename Hueing>
TerminalBackend<Hueing>::TerminalBackend(
    const LiveBackend<Hueing>& other, Status& s )
: image( other.image ),
  colorizer( other.colorizer ),
  discretization( other.discretization, image(), colorizer ),
  cache(),
  status( s )
{
    if ( other.cia.getSize().is_initialized() )
        cache.setSize( *other.cia.getSize() );
    image.setListener(&discretization);
    discretization.setListener(&cache);
}

template <typename Hueing>
std::auto_ptr<Backend>
LiveBackend<Hueing>::adapt( std::auto_ptr<Backend> self, Status& s ) {
    assert( self.get() == this );

    if ( ! s.config.showOutput() ) {
        std::auto_ptr<Backend> fresh( new TerminalBackend<Hueing>(*this, s) );
        std::swap( fresh, self );
        /* Self is now destructed! Take care not to modify or access this. */
    }

    return self;
}

}
}

#endif
