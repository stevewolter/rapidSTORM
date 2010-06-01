#ifndef dStorm_outputs_BinnedLocalizations_converter_h
#define dStorm_outputs_BinnedLocalizations_converter_h
#include "BinnedLocalizations.h"

namespace dStorm {
namespace outputs {

template <typename Listener>
template <typename OtherListener>
BinnedLocalizations<Listener>::
    BinnedLocalizations(const BinnedLocalizations<OtherListener>& o)
: OutputObject("BinnedLocalizations", ""),
  re(o.re),
  crop(o.crop),
  base_image(o.base_image),
  announcement( (o.announcement.get()) ? new Announcement(*o.announcement) : NULL )
{
}

}
}

#endif
