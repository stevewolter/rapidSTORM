#include "debug.h"
#include "estimate_psf_form/LocalizationValueFinder.h"
#include "guf/TraitValueFinder.h"
#include "Localization.h"
#include "fit_window/Optics.h"
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include "gaussian_psf/DepthInfo3D.h"

namespace dStorm {
namespace estimate_psf_form {

namespace PSF = gaussian_psf;

/* TODO: This class is not Laempi- or disjoint-aware */
struct LocalizationValueFinder::application {
    typedef void result_type;
    const Localization& parent, *child;
    fit_window::Detector optics;
    guf::TraitValueFinder tvf;
    const dStorm::traits::Optics& plane;

    application( 
        const int fluorophore, const dStorm::traits::Optics& plane,
        const Localization& parent, size_t plane_number )
        : parent(parent), 
          child( 
                (parent.children.is_initialized() && parent.children->size() > plane_number )
                ? &(*parent.children)[plane_number] : NULL ),
          optics( plane ),
          tvf(fluorophore,plane),
          plane( plane ) {}

    template <int Dim, typename Structure>
    void operator()( nonlinfit::Xs<Dim>, Structure& ) const {}
    template <int Dim, typename Structure>
    void operator()( PSF::Mean<Dim> p, Structure& m ) const
        { m(p) = parent.position()[Dim] / (1E-6 * si::meter); }
    template <typename Structure>
    void operator()( PSF::MeanZ p, Structure& m ) const
        { m(p) = parent.position()[2] / (1E-6 * si::meter); }
    template <typename Structure>
    void operator()( PSF::Amplitude p, Structure& m ) const { 
        m(p) = optics.relative_in_photons( parent.amplitude() ); 
    }
    template <typename Structure>
    void operator()( constant_background::Amount p, Structure& m ) const
        { m(p) = optics.absolute_in_photons( ( child ) ? child->local_background() : parent.local_background() ); }

    /** This overload is responsible to set the transmission 
     *  coefficient to a positive value. This avoids zero
     *  gradients for transmission coefficients. */
    template <typename Structure>
    void operator()( PSF::Prefactor p, Structure& m ) const { 
        tvf( p, m );  
        if ( m(p) < 1E-3 )
            m(p) = 1E-3;
    }

    template <typename Parameter, typename Structure>
    void operator()( Parameter p, Structure& m ) const { 
        tvf( p, m );  
    }
};

LocalizationValueFinder::LocalizationValueFinder(
        const int fluorophore, const dStorm::traits::Optics& plane,
        const Localization& parent, size_t plane_number )
: appl_( new application(fluorophore,plane,parent,plane_number) ) {}

void LocalizationValueFinder::find_values( gaussian_psf::DepthInfo3D& z ) {
    boost::mpl::for_each< typename gaussian_psf::DepthInfo3D::Variables >( 
        boost::bind( boost::ref(*appl_), _1, boost::ref(z) ) );
    z.set_spline( appl_->plane.depth_info(Direction_X), appl_->plane.depth_info(Direction_Y) );
}

template <typename Type>
void LocalizationValueFinder::find_values_( Type& z ) {
    boost::mpl::for_each< typename Type::Variables >( 
        boost::bind( boost::ref(*appl_), _1, boost::ref(z) ) );
}

template void LocalizationValueFinder::find_values_<gaussian_psf::No3D>( gaussian_psf::No3D& );
template void LocalizationValueFinder::find_values_<constant_background::Expression>( constant_background::Expression& );
}
}
