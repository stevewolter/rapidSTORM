#ifndef dStorm_outputs_BinnedLocalizations_converter_h
#define dStorm_outputs_BinnedLocalizations_converter_h
#include "BinnedLocalizations.h"

namespace dStorm {
namespace outputs {

template <typename Listener, int Dim>
template <typename OtherListener>
BinnedLocalizations<Listener,Dim>::
    BinnedLocalizations(const BinnedLocalizations<OtherListener,Dim>& o)
: crop(o.crop),
  base_image(o.base_image),
  announcement( (o.announcement.get()) ? new Announcement(*o.announcement) : NULL ),
  strategy( o.strategy->clone() ),
  binningInterpolator( o.binningInterpolator->clone() )
{
}

}
}

#endif
