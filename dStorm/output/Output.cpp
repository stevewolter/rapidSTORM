#include "Output.h"

namespace dStorm {
namespace output {

void Output::attach_ui( simparm::NodeHandle at ) {
    attach_ui_(at);
}

Output::Announcement::Announcement( 
    const input::Traits<LocalizedImage>& traits )
    : input::Traits<LocalizedImage>(traits) {}

}
}
