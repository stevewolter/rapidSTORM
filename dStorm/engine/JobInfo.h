#ifndef DSTORM_ENGINE_JOBINFO_H
#define DSTORM_ENGINE_JOBINFO_H

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/systems/camera/intensity.hpp>
#include "Image_decl.h"

namespace dStorm {
namespace engine {

struct JobInfo {
    typedef boost::units::quantity<boost::units::camera::intensity> Intensity;
    Intensity amplitude_threshold;
    const InputTraits& traits;
    int fluorophore;

    JobInfo( Intensity amp_thres, const InputTraits& i, int fluorophore )
        : amplitude_threshold(amp_thres), traits(i), fluorophore(fluorophore) {}
    JobInfo( const JobInfo& o, const InputTraits& t )
        : amplitude_threshold(o.amplitude_threshold),
          traits(t), fluorophore(o.fluorophore) {}
};


}
}

#endif
