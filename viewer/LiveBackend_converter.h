#ifndef DSTORM_VIEWER_LIVEBACKEND_CONVERTER_H
#define DSTORM_VIEWER_LIVEBACKEND_CONVERTER_H

#include <dStorm/outputs/BinnedLocalizations.h>
#include "ImageDiscretizer_converter.h"
#include "Display.h"
#include "TerminalBackend.h"
#include "LiveBackend.h"
#include "Status_decl.h"
#include "Status.h"

namespace dStorm {
namespace viewer {

template <typename Hueing>
LiveBackend<Hueing>::LiveBackend(const TerminalBackend<Hueing>& other, Status& s)
: status(s), 
  image( other.image ),
  colorizer( other.colorizer ),
  discretization( other.discretization,
                  image(), colorizer ),
  cache( 4096, other.cache.getSize().size.template head<Im::Dim>() ),
  cia( discretization, s, *this, colorizer, other.get_result() )
{
    cia.set_job_name( other.get_job_name() );
    image.setListener(&discretization);
    discretization.setListener(&cache);
    cache.setListener(&cia);
}

template <typename Hueing>
std::auto_ptr<Backend>
TerminalBackend<Hueing>::adapt( std::auto_ptr<Backend> self, Status& s ) {
    assert( self.get() == this );

    if ( s.config.showOutput() ) {
        std::auto_ptr<Backend> fresh( new LiveBackend<Hueing>(*this, s) );
        std::swap( fresh, self );
        /* Self is now destructed! Take care not to modify or access this. */
    }

    return self;
}

}
}

#endif
