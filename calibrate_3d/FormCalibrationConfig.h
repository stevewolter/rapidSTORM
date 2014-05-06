#ifndef DSTORM_CALIBRATE3D_FORMCALIBRATIONCONFIG_H
#define DSTORM_CALIBRATE3D_FORMCALIBRATIONCONFIG_H

#include <string>
#include <boost/array.hpp>
#include <simparm/Entry.h>
#include "polynomial_3d.h"
#include "Direction.h"

namespace dStorm {
namespace calibrate_3d {

struct FormCalibrationConfig {
    boost::array< boost::optional< simparm::BoolEntry >, polynomial_3d::Order > z_terms;
    simparm::Entry< bool > circular_psf, astigmatism, universal_best_sigma, universal_prefactors;
    simparm::Entry< bool > fit_best_sigma, fit_focus_plane, fit_prefactors;

    FormCalibrationConfig();
    void register_generic_entries( simparm::NodeHandle at );
    void register_multiplane_entries( simparm::NodeHandle at );
    void register_polynomial3d_entries( simparm::NodeHandle at );

    bool fit_z_term( Direction, polynomial_3d::Term term ) const
        { return (*z_terms[ polynomial_3d::offset(term) ])(); }
    bool symmetric() const { return circular_psf(); }
    bool fit_transmission() const { return fit_prefactors(); }
    bool universal_3d() const { return universal_prefactors(); }
};

}
}

#endif
