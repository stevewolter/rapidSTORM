#ifndef DSTORM_VIEWER_TERMBACKEND_CONVERTER_H
#define DSTORM_VIEWER_TERMBACKEND_CONVERTER_H

#include <dStorm/outputs/BinnedLocalizations.h>
#include "ImageDiscretizer_converter.h"
#include "ImageDiscretizer_inline.h"
#include "Display.h"
#include "LiveBackend.h"
#include "TerminalBackend.h"
#include "Config.h"
#include "Status.h"

namespace dStorm {
namespace viewer {

TerminalBackend::TerminalBackend(
    const LiveBackend<ColourScheme>& other, Status& s )
: image( NULL, other.image ),
  colorizer( other.colorizer->clone() ),
  discretization( other.discretization, image(), *colorizer ),
  cache(),
  status( s )
{
    if ( other.cia.getSize().is_initialized() )
        cache.setSize( *other.cia.getSize() );
    image.set_listener(&discretization);
    discretization.setListener(&cache);
}

template <typename Hueing>
std::auto_ptr<Backend>
LiveBackend<Hueing>::change_liveness( Status& s ) {
    if ( ! s.config.showOutput() ) {
        return std::auto_ptr<Backend>( new TerminalBackend(*this, s) );
    } else {
        return std::auto_ptr<Backend>();
    }
}

}
}

#endif
