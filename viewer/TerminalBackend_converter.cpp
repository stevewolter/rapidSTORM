#include "TerminalBackend_converter.h"
#include "LiveBackend.h"
#include "ColourDisplay_impl.h"
#include "Status_decl.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) template \
    std::auto_ptr<Backend> LiveBackend<Hueing> \
        ::adapt( std::auto_ptr<Backend> self, Config& c, Status& s )

DISC_INSTANCE(ColourSchemes::BlackWhite);
DISC_INSTANCE(ColourSchemes::BlackRedYellowWhite);
DISC_INSTANCE(ColourSchemes::FixedHue);
DISC_INSTANCE(ColourSchemes::TimeHue);
DISC_INSTANCE(ColourSchemes::ExtraHue);
DISC_INSTANCE(ColourSchemes::ExtraSaturation);

}
}
