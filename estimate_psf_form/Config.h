#ifndef DSTORM_FORMFITTER_CONFIG_H
#define DSTORM_FORMFITTER_CONFIG_H

#include <boost/units/Eigen/Array>
#include <simparm/Eigen_decl.h>
#include <simparm/BoostUnits.h>
#include <simparm/Eigen.h>
#include <simparm/Object.h>
#include <simparm/Entry.h>
#include <simparm/FileEntry.h>
#include <simparm/Group.h>
#include "output/Capabilities.h"
#include <boost/array.hpp>
#include <boost/optional/optional.hpp>
#include "Direction.h"
#include "units/nanolength.h"
#include <memory>

#include "calibrate_3d/FormCalibrationConfig.h"

namespace dStorm {
namespace estimate_psf_form {

using boost::units::quantity;
namespace si = boost::units::si;

struct Config : public calibrate_3d::FormCalibrationConfig
{
    simparm::Group multiplane;
    simparm::BoolEntry mle;
    simparm::Entry<unsigned long> number_of_spots; 
    simparm::Entry<double> max_per_image;
    simparm::BoolEntry visual_selection, laempi_fit, disjoint_amplitudes;
    typedef Eigen::Matrix< quantity<si::nanolength>, 2, 1, Eigen::DontAlign > FitWindowWidth;
    simparm::Entry< FitWindowWidth > fit_window_width;

    Config();
    void attach_ui( simparm::NodeHandle at );

    static std::string get_name() { return "FitPSFForm"; }
    static std::string get_description() { return "Estimate PSF form"; }
    static simparm::UserLevel get_user_level() { return simparm::Intermediate; }

    bool can_work_with(output::Capabilities cap)  {
            return cap.test( output::Capabilities::SourceImage );
    }
};

}
}

#endif
