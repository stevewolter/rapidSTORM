#ifndef DSTORM_OUTPUT_TRAITS_H
#define DSTORM_OUTPUT_TRAITS_H

#include <simparm/optional.hh>
#include "Traits_decl.h"
#include <dStorm/SizeTraits.h>
#include <dStorm/Localization.h>

#include <boost/units/unit.hpp>
#include <boost/units/systems/si/time.hpp>

namespace dStorm {
namespace output {

class Traits 
: public SizeTraits<Localization::Dim> 
{
  public:
    Traits() {}
    Traits( const SizeTraits<Localization::Dim>& t );

    simparm::optional<boost::units::si::time> frame_length;
    simparm::optional<unsigned int> total_frame_count;
};

}
}

#endif
