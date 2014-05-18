#include "namespaces.h"
#include "simparm/Entry.h"
#include <boost/units/systems/camera/intensity.hpp>

namespace dStorm {

    typedef simparm::Entry< boost::units::quantity< camera::intensity, float > > 
        ADCEntry;
}
