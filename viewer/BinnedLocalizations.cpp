#include <dStorm/outputs/BinnedLocalizations_impl.h>
#include "ImageDiscretizer_inline.h"
#include "LiveCache_inline.h"
#include "TerminalCache_inline.h"
#include "Display_inline.h"
#include "colour_schemes/impl.h"

namespace dStorm {
namespace outputs {

#define DISC_INSTANCE(Hueing) \
    template class \
        BinnedLocalizations< \
            viewer::Discretizer<\
                viewer::LiveCache<\
                    viewer::Display<\
                        viewer::Hueing > > > >; \
    template class \
        BinnedLocalizations< \
            viewer::Discretizer<\
                viewer::TerminalCache<\
                    viewer::Hueing > > > \

#include "colour_schemes/instantiate.h"

}
}
