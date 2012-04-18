#ifndef DSTORM_FITTER_GUF_TRAIT_VALUE_FINDER_H
#define DSTORM_FITTER_GUF_TRAIT_VALUE_FINDER_H

#include "FitAnalysis.h"
#include "Config.h"
#include <dStorm/engine/Input.h>
#include <dStorm/traits/optics.h>
#include <dStorm/engine/InputTraits.h>
#include <boost/variant/get.hpp>
#include "guf/psf/parameters.h"
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
    void operator()( PSF::BestSigma<Dim> p, PSF::No3D& m ) const { 
        m(p) = quantity< typename PSF::Micrometers >(
            dynamic_cast< const threed_info::No3D&>(*plane.depth_info())
                .sigma[Dim] );
    }
    template <int Dim>
    void operator()( PSF::BestSigma<Dim> p, PSF::Polynomial3D& m ) const { 
        m(p) = quantity< typename PSF::Micrometers >(
            dynamic_cast< const threed_info::Polynomial3D&>(*plane.depth_info())
                .get_base_width( static_cast<Direction>(Dim) ) );
    }
    template <int Dim, typename Structure, int Term>
    void operator()( PSF::DeltaSigma<Dim,Term> p, Structure& m ) const {
        m(p) = quantity< typename PSF::Micrometers >(
            dynamic_cast<const threed_info::Polynomial3D&>(*plane.depth_info())
                .get_slope( static_cast<dStorm::Direction>(Dim), Term ) );
    }

    template <int Dim, typename Structure>
    void operator()( PSF::ZPosition<Dim> p, Structure& m ) const { 
        m( p ) = quantity< typename PSF::ZPosition<Dim>::Unit >(
            dynamic_cast<const threed_info::Polynomial3D&>(*plane.depth_info())
                .focal_planes()->coeff(Dim,0) );
    }

    template <typename Structure>
    void operator()( PSF::Prefactor  p, Structure& m ) const {
        m(p) = plane.transmission_coefficient(fluorophore); 
    }
};

}
}

#endif
