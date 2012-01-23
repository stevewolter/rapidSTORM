#include "Output.h"

namespace boost {
template <>
dStorm::output::Output* new_clone<dStorm::output::Output>
    ( const dStorm::output::Output& o )
{
    return o.clone();
}

template <>
void delete_clone<dStorm::output::Output>(const dStorm::output::Output* o)
{
    delete o;
}

}

namespace dStorm {
namespace output {

Output::Announcement::Announcement( 
    const input::Traits<LocalizedImage>& traits,
    display::Manager& manager )
    : input::Traits<LocalizedImage>(traits), manager(&manager) {}

}
}
