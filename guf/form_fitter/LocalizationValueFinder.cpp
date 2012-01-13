#include "LocalizationValueFinder.h"
#include "guf/guf/TraitValueFinder.h"
#include <dStorm/Localization.h>
#include "guf/guf/mle_converter.h"
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>

namespace dStorm {
namespace form_fitter {

namespace PSF = guf::PSF;

/* TODO: This class is not Laempi- or disjoint-aware */
struct LocalizationValueFinder::application {
    typedef void result_type;
    const Localization& parent, *child;
    guf::mle_converter mle;
    guf::TraitValueFinder tvf;

    application( 
        const dStorm::engine::JobInfo& info, const dStorm::traits::Optics<2>& plane,
        const Localization& parent, size_t plane_number )
        : parent(parent), 
          child( 
                (parent.children.is_initialized() && parent.children->size() > plane_number )
                ? &(*parent.children)[plane_number] : NULL ),
          mle( plane ),
          tvf(info,plane) {}

    template <int Dim, typename Structure>
    void operator()( nonlinfit::Xs<Dim,PSF::LengthUnit>, Structure& ) const {}
    template <int Dim, typename Structure>
    void operator()( PSF::Mean<Dim> p, Structure& m ) const
        { m(p) = parent.position()[Dim]; }
    template <typename Structure>
    void operator()( PSF::MeanZ p, Structure& m ) const
        { m(p) = parent.position()[2]; }
    template <typename Structure>
    void operator()( PSF::Amplitude p, Structure& m ) const
        { m(p) = mle.convert_amplitude( parent.amplitude() ); }
    template <typename Structure>
    void operator()( constant_background::Amount p, Structure& m ) const
        { m(p) = mle.convert_shift( ( child ) ? child->local_background() : parent.local_background() ); }

    template <typename Parameter, typename Structure>
    void operator()( Parameter p, Structure& m ) const { 
        tvf( p, m );  
    }
};

LocalizationValueFinder::LocalizationValueFinder(
        const dStorm::engine::JobInfo& info, const dStorm::traits::Optics<2>& plane,
        const Localization& parent, size_t plane_number )
: appl_( new application(info,plane,parent,plane_number) ) {}

template <typename Type>
void LocalizationValueFinder::find_values_( Type& z ) {
    boost::mpl::for_each< typename Type::Variables >( 
        boost::bind( boost::ref(*appl_), _1, boost::ref(z) ) );
}

template void LocalizationValueFinder::find_values_<guf::PSF::Zhuang>( guf::PSF::Zhuang& );
template void LocalizationValueFinder::find_values_<guf::PSF::No3D>( guf::PSF::No3D& );
template void LocalizationValueFinder::find_values_<constant_background::Expression>( constant_background::Expression& );
}
}
