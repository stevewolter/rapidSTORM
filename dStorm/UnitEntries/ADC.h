#include "../namespaces.h"
#include <simparm/Entry.hh>
#include <boost/units/systems/camera/intensity.hpp>

namespace dStorm {

    typedef simparm::Entry< boost::units::quantity< camera::intensity, float > > 
        ADCEntry;
}
