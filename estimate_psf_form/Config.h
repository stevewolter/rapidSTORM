#ifndef DSTORM_FORMFITTER_CONFIG_H
#define DSTORM_FORMFITTER_CONFIG_H

#include <boost/units/Eigen/Array>
#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <simparm/FileEntry.hh>
#include <simparm/Set.hh>
#include <dStorm/output/Capabilities.h>
#include <boost/array.hpp>
#include <boost/optional/optional.hpp>
#include <dStorm/polynomial_3d.h>
#include <dStorm/Direction.h>
#include <dStorm/units/nanolength.h>
#include <memory>

#include "calibrate_3d/FormCalibrationConfig.h"

namespace dStorm {
namespace estimate_psf_form {

using boost::units::quantity;
namespace si = boost::units::si;

struct Config : public calibrate_3d::FormCalibrationConfig
{
    simparm::Set multiplane, polynomial_3d;
    simparm::BoolEntry mle;
    simparm::Entry<unsigned long> number_of_spots; 
    simparm::Entry<double> max_per_image;
    simparm::BoolEntry visual_selection, laempi_fit, disjoint_amplitudes, z_is_truth;
    typedef Eigen::Matrix< quantity<si::nanolength>, 2, 1, Eigen::DontAlign > FitWindowWidth;
    simparm::Entry< FitWindowWidth > fit_window_width;

    Config();
    void attach_ui( simparm::Node& at );

    static std::string get_name() { return "FitPSFForm"; }
    static std::string get_description() { return "Estimate PSF form"; }

    bool can_work_with(output::Capabilities cap)  {
            return cap.test( output::Capabilities::SourceImage );
    }
};

}
}

#endif
