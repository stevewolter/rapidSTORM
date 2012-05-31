#ifndef DSTORM_FITTER_GUF_TRAIT_VALUE_FINDER_H
#define DSTORM_FITTER_GUF_TRAIT_VALUE_FINDER_H

#include "MultiKernelModel.h"
#include "Config.h"
#include <dStorm/engine/Input.h>
#include <dStorm/traits/optics.h>
#include <dStorm/engine/InputTraits.h>
#include <boost/variant/get.hpp>
#include "gaussian_psf/parameters.h"
#include <limits>
#include <dStorm/threed_info/No3D.h>
#include <dStorm/threed_info/Polynomial3D.h>

namespace dStorm {
namespace guf {

struct TraitValueFinder {
    const int fluorophore;
    const dStorm::traits::Optics& plane;

  public:
    typedef void result_type;
    TraitValueFinder( const int fluorophore, const dStorm::traits::Optics& plane );

    template <int Dim>
    void operator()( gaussian_psf::BestSigma<Dim> p, gaussian_psf::No3D& m ) const { 
        Direction d = static_cast<Direction>(Dim);
        m(p) = quantity< typename gaussian_psf::Micrometers >(
            dynamic_cast< const threed_info::No3D&>(*plane.depth_info(d))
                .sigma );
    }
    template <int Dim>
    void operator()( gaussian_psf::BestSigma<Dim> p, gaussian_psf::Polynomial3D& m ) const { 
        Direction d = static_cast<Direction>(Dim);
        m(p) = quantity< typename gaussian_psf::Micrometers >(
            dynamic_cast< const threed_info::Polynomial3D&>(*plane.depth_info(d))
                .get_base_width() );
    }
    template <int Dim, typename Structure, int Term>
    void operator()( gaussian_psf::DeltaSigma<Dim,Term> p, Structure& m ) const {
        Direction d = static_cast<Direction>(Dim);
        m(p) = quantity< typename gaussian_psf::Micrometers >(
            dynamic_cast<const threed_info::Polynomial3D&>(*plane.depth_info(d))
                .get_slope( Term ) );
    }

    template <int Dim, typename Structure>
    void operator()( gaussian_psf::ZPosition<Dim> p, Structure& m ) const { 
        Direction d = static_cast<Direction>(Dim);
        m( p ) = quantity< typename gaussian_psf::ZPosition<Dim>::Unit >(
            dynamic_cast<const threed_info::Polynomial3D&>(*plane.depth_info(d))
                .focal_plane() );
    }

    template <typename Structure>
    void operator()( nonlinfit::Xs<2,gaussian_psf::LengthUnit> p, Structure& m ) const { 
        m( p ) = quantity<gaussian_psf::LengthUnit>(0E-6 * si::metre);
    }

    template <typename Structure>
    void operator()( gaussian_psf::Prefactor  p, Structure& m ) const {
        m(p) = plane.transmission_coefficient(fluorophore); 
    }
};

}
}

#endif
