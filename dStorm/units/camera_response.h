#include <boost/units/quantity.hpp>
#include <cs_units/camera/intensity.hpp>

namespace dStorm {
    typedef boost::units::quantity<cs_units::camera::intensity,float> camera_response;
}
