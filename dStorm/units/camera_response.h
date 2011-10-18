#include <boost/units/quantity.hpp>
#include <boost/units/systems/camera/intensity.hpp>

namespace dStorm {
    typedef boost::units::quantity<boost::units::camera::intensity,float> camera_response;
}
