#ifndef DSTORM_FORMFITTER_CONFIG_H
#define DSTORM_FORMFITTER_CONFIG_H

#include <boost/units/Eigen/Array>
#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/output/Capabilities.h>
#include <boost/array.hpp>
#include <boost/optional/optional.hpp>
#include <dStorm/polynomial_3d.h>
#include <dStorm/Direction.h>
#include <dStorm/units/nanolength.h>
#include <memory>

#include "calibrate_3d/FormCalibrationConfig.h"

namespace dStorm {
namespace form_fitter {

using boost::units::quantity;
namespace si = boost::units::si;

struct Config : public simparm::Object, public calibrate_3d::FormCalibrationConfig
{
    simparm::BoolEntry auto_disable, mle;
    simparm::Entry<unsigned long> number_of_spots; 
    simparm::Entry<double> width_correction, max_per_image;
    simparm::BoolEntry visual_selection, 
                        laempi_fit, disjoint_amplitudes;
    typedef Eigen::Matrix< quantity<si::nanolength>, 2, 1, Eigen::DontAlign > FitWindowWidth;
    simparm::Entry< FitWindowWidth > fit_window_width;

    Config();
    void registerNamedEntries();

    bool can_work_with(output::Capabilities cap)  {
            return cap.test( output::Capabilities::SourceImage );
    }
};

}
}

#endif
