#include "Localization.h"
#include "input/LocalizationTraits.h"
#include <boost/units/io.hpp>
#include <boost/fusion/include/iteration.hpp>
#include <Eigen/Array>

namespace dStorm {

struct spacesep_output_streamer {
    std::ostream& l;
    mutable bool first;
    spacesep_output_streamer(std::ostream& l) : l(l), first(true) {}

    template <typename Type>
        std::ostream& operator()( Type& type ) const
        { if ( first ) first = false; else l << " "; return (l << type()); }
};

std::ostream&
operator<<(std::ostream &o, const Localization& loc)
{
    boost::fusion::for_each( loc, spacesep_output_streamer(o) );
    return o << "\n";
}

}
