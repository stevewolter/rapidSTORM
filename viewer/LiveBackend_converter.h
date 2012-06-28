#ifndef DSTORM_VIEWER_LIVEBACKEND_CONVERTER_H
#define DSTORM_VIEWER_LIVEBACKEND_CONVERTER_H

#include <dStorm/outputs/BinnedLocalizations.h>
#include "LiveCache_inline.h"
#include "ImageDiscretizer_converter.h"
#include "ImageDiscretizer_inline.h"
#include "Display.h"
#include "Display_inline.h"
#include "TerminalBackend.h"
#include "LiveBackend.h"
#include "Status_decl.h"
#include "Status.h"

namespace dStorm {
namespace viewer {

template <typename Hueing>
LiveBackend<Hueing>::LiveBackend(const TerminalBackend& other, Status& s)
: status(s), 
  image( NULL, other.image ),
  colorizer( other.colorizer->clone() ),
  discretization( other.discretization,
                  image(), *colorizer ),
  cache( 4096, other.cache.getSize().size ),
  cia( discretization, s, *this, *colorizer, other.get_result() )
{
    cia.set_job_name( other.get_job_name() );
    image.set_listener(&discretization);
    discretization.setListener(&cache);
    cache.setListener(&cia);
    cia.show_window();
}

}
}

#endif
