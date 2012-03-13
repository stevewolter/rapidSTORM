#ifndef DSTORM_CALIBRATE3D_CONSTANT_PARAMETER_HPP
#define DSTORM_CALIBRATE3D_CONSTANT_PARAMETER_HPP

#include "FormCalibrationConfig.h"
#include "guf/psf/parameters.h"
#include "guf/constant_background_fwd.hpp"

namespace dStorm {
namespace calibrate_3d {

class constant_parameter {
    const FormCalibrationConfig& config;
    bool multiplane;

public:
    constant_parameter( bool has_multiple_planes, const FormCalibrationConfig& config )
        : config(config), multiplane(has_multiple_planes) {}

    typedef bool result_type;
    template <typename Func, typename Base>
    bool operator()( nonlinfit::TermParameter< Func, Base > ) { return (*this)( Base() ); }

    template <int Dim> 
    bool operator()( nonlinfit::Xs<Dim,guf::PSF::LengthUnit> ) { return true; }
    template <int Dim> 
    bool operator()( guf::PSF::Mean<Dim> ) { return false; }
    bool operator()( guf::PSF::Amplitude ) { return false; }
    bool operator()( constant_background::Amount ) { return false; }
    template <int Dim> 
    bool operator()( guf::PSF::ZPosition<Dim> ) { return ! config.fit_focus_plane(); }
    template <int Dim> 
    bool operator()( guf::PSF::BestSigma<Dim> ) { return ! config.fit_best_sigma(); }
    bool operator()( guf::PSF::MeanZ ) { return config.has_z_truth(); }
    template <int Dim, int Term>
    bool operator()( guf::PSF::DeltaSigma<Dim,Term> ) { return ! config.fit_z_term(static_cast<Direction>(Dim), Term); }
    bool operator()( guf::PSF::Prefactor ) { return ! multiplane || ! config.fit_transmission(); }
};

}
}

#endif
