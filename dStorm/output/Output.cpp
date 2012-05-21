#include "Output.h"

namespace dStorm {
namespace output {

void Output::attach_ui( simparm::Node& at ) {
    attach_ui_(at);
}

Output::Announcement::Announcement( 
    const input::Traits<LocalizedImage>& traits,
    display::Manager& manager )
    : input::Traits<LocalizedImage>(traits), manager(&manager) {}

}
}
