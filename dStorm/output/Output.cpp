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
