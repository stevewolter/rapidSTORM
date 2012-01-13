#include <Eigen/StdVector>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/ImageTraits.h>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include "InitialValueFinder.h"
#include "guf/psf/expressions.h"
#include "guf/constant_background.hpp"
#include "TraitValueFinder.h"

namespace dStorm {
namespace guf {

InitialValueFinder::InitialValueFinder( const Config& config, const dStorm::engine::JobInfo& info) 
: info(info),
  disjoint_amplitudes( config.disjoint_amplitudes() )
{
}

class InitialValueFinder::set_parameter {
    TraitValueFinder base;
    const InitialValueFinder& p;
    const guf::Spot& s;
    const PlaneEstimate& e;

  public:
    typedef void result_type;
    set_parameter( const InitialValueFinder& p, const guf::Spot& s, const PlaneEstimate& e, const dStorm::traits::Optics<2>& o ) 
        : base( p.info, o ), p(p), s(s), e(e) {}

    template <typename Model>
    void operator()( nonlinfit::Xs<0,PSF::LengthUnit> p, Model& m ) {}
    template <typename Model>
    void operator()( nonlinfit::Xs<1,PSF::LengthUnit> p, Model& m ) {}
    template <int Dim, typename Model>
    void operator()( PSF::Mean<Dim> p, Model& m ) 
        { m( p ) = s[Dim]; }
    void operator()( PSF::MeanZ p, PSF::Zhuang& m ) 
        { m( p ) = 1E-10 * boost::units::si::meter; }
    template <typename Model>
    void operator()( PSF::Amplitude a, Model& m ) 
        { m( a ) = e.amp; }
    void operator()( constant_background::Amount a, constant_background::Expression& m ) 
        { m( a ) = e.bg; }
    template <typename Parameter, typename Model>
    void operator()( Parameter p, Model& m ) { base( p, m ); }
};

void InitialValueFinder::operator()( 
    FitPosition& position, 
    const guf::Spot& spot,
    const guf::Statistics<3>& data
) const {
    std::vector<PlaneEstimate> e = estimate_bg_and_amp(spot,data);
    if ( ! disjoint_amplitudes ) join_amp_estimates( e );

    typedef boost::tuple<
            FitPosition::iterator,
            std::vector<PlaneEstimate>::const_iterator,
            dStorm::traits::Optics<3>::Planes::const_iterator >
        the_iterator_tuple;
    typedef boost::zip_iterator< the_iterator_tuple >
        the_zip_iterator;

    the_zip_iterator i, end = the_zip_iterator( the_iterator_tuple( position.end(), e.end(), info.traits.planes.end() ) );
    for ( i = the_zip_iterator( the_iterator_tuple(position.begin(), e.begin(), info.traits.planes.begin()) ); i != end; ++i )
    {
        assert( ( i->get<0>().kernel_count() ) == 1 );
        set_parameter s( *this, spot, i->get<1>(), i->get<2>() );
        if ( PSF::Zhuang* z = dynamic_cast<PSF::Zhuang*>(&i->get<0>()[0]) )
            boost::mpl::for_each< PSF::Zhuang::Variables >( 
                boost::bind( boost::ref(s), _1, boost::ref( *z ) ) );
        else if ( PSF::No3D* z = dynamic_cast<PSF::No3D*>(&i->get<0>()[0]) )
            boost::mpl::for_each< PSF::No3D::Variables >( 
                boost::bind( boost::ref(s), _1, boost::ref( *z ) ) );
        else
            throw std::logic_error("Somebody forgot a 3D model in " + std::string(__FILE__) );
        s( constant_background::Amount(), i->get<0>().background_model() );
    }
}

void InitialValueFinder::join_amp_estimates( std::vector<PlaneEstimate>& v ) const {
    int used_planes = 0;
    double mean_amplitude = 0;
    for (int i = 0; i < int(v.size()); ++i) {
        if ( v[i].amp > 0 ) {
            mean_amplitude += v[i].amp;
            ++used_planes;
        }
    }
    mean_amplitude /= used_planes;
    for (int i = 0; i < int(v.size()); ++i)
        v[i].amp = mean_amplitude;
}


std::vector<InitialValueFinder::PlaneEstimate> InitialValueFinder::estimate_bg_and_amp( 
    const guf::Spot&,
    const guf::Statistics<3> & s
) const {
    std::vector<PlaneEstimate> rv( s.size() );
    for (int i = 0; i < int(s.size()); ++i) {
        /* Value of perfectly sharp PSF at Z = 0 */
        double pif = info.traits.plane(i).transmission_coefficient(info.fluorophore);
            
        /* Solution of equation system:
            * peak_intensity == bg_estimate + pif * amp_estimate
            * integral == pixel_count * bg_estimate + amp_estimate */
        if ( pif > 1E-20 ) {
            rv[i].bg = s[i].quarter_percentile_pixel.value();
            rv[i].amp = std::max( 
                (s[i].integral.value() - rv[i].bg * double(s[i].pixel_count)) / pif,
                1.0 );
        } else {
            rv[i].amp = 0;
            rv[i].bg = s[i].integral.value() / s[i].pixel_count;
        }
    }
    return rv;
}

}
}
