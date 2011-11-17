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
    double mask_size_factor;
    Intensity amplitude_threshold;
    const InputTraits& traits;
    int fluorophore;

    JobInfo( float msf, Intensity amp_thres, const InputTraits& i, int fluorophore )
        : mask_size_factor(msf), amplitude_threshold(amp_thres), traits(i), fluorophore(fluorophore) {}
    JobInfo( const JobInfo& o, const InputTraits& t )
        : mask_size_factor(o.mask_size_factor), amplitude_threshold(o.amplitude_threshold),
          traits(t), fluorophore(o.fluorophore) {}

    int mask_size( int dimension, int plane ) const;
    boost::units::quantity< boost::units::camera::length > sigma( int dimension, int plane ) const;
    boost::units::quantity< boost::units::si::length > sigma_in_si( int dimension, int plane ) const;
    boost::units::quantity< boost::units::si::length > mask_size_in_si( int dimension, int plane ) const;
};


}
}

#endif
