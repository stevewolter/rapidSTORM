#ifndef DSTORM_CALIBRATE3D_FORMCALIBRATIONCONFIG_H
#define DSTORM_CALIBRATE3D_FORMCALIBRATIONCONFIG_H

#include <string>
#include <boost/array.hpp>
#include <simparm/Entry.hh>
#include <dStorm/polynomial_3d.h>
#include <dStorm/Direction.h>

namespace dStorm {
namespace calibrate_3d {

class FormCalibrationConfig {
    boost::array< boost::optional< simparm::BoolEntry >, polynomial_3d::Order > z_terms;
    simparm::Entry< bool > circular_psf_, astigmatism_, universal_best_sigma_, universal_prefactors_;
    simparm::Entry< bool > fit_best_sigma_, fit_focus_plane_, fit_prefactors_;
public:
    FormCalibrationConfig();
    void register_generic_entries( simparm::NodeHandle at );
    void register_multiplane_entries( simparm::NodeHandle at );
    void register_polynomial3d_entries( simparm::NodeHandle at );

    bool fit_z_term( Direction, polynomial_3d::Term term ) const
        { return (*z_terms[ polynomial_3d::offset(term) ])(); }
    bool symmetric() const { return circular_psf_(); }
    bool astigmatism() const { return astigmatism_(); }
    bool universal_best_sigma() const { return universal_best_sigma_(); }
    bool universal_3d() const { return universal_prefactors_(); }
    bool fit_best_sigma() const { return fit_best_sigma_(); }
    bool fit_focus_plane() const { return fit_focus_plane_(); }
    bool fit_transmission() const { return fit_prefactors_(); }
};

}
}

#endif
