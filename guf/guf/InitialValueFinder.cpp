#include "debug.h"
#include <Eigen/StdVector>
#include <dStorm/engine/JobInfo.h>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include "InitialValueFinder.h"
#include "guf/psf/expressions.h"
#include "guf/constant_background.hpp"
#include "TraitValueFinder.h"
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <dStorm/threed_info/look_up_sigma_diff.h>
#include <boost/accumulators/statistics/covariance.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/variates/covariate.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/framework/accumulator_set.hpp>

using namespace boost::accumulators;

namespace dStorm {
namespace guf {

struct InitialValueFinder::PlaneEstimate { 
    double bg; 
    double amp; 
    boost::units::quantity<boost::units::si::length> z_estimate; 
};

threed_info::SigmaDiffLookup InitialValueFinder::SigmaDiff::lookup( const dStorm::engine::JobInfo& info ) const {
    return threed_info::SigmaDiffLookup(
        *info.traits.optics( minuend_plane ).depth_info(),
        minuend_dir,
        *info.traits.optics( subtrahend_plane ).depth_info(),
        subtrahend_dir );
}

InitialValueFinder::InitialValueFinder( const Config& config, const dStorm::engine::JobInfo& info) 
: info(info),
  disjoint_amplitudes( config.disjoint_amplitudes() )
{
    float max_corr = -1;
    const int dim_count = Direction_2D * info.traits.plane_count();
    for (int dim1 = 0; dim1 < dim_count; ++dim1)
        for (int dim2 = dim1+1; dim2 < dim_count; ++dim2) {
            SigmaDiff diff;
            diff.minuend_plane = dim1 / Direction_2D;
            diff.minuend_dir = Direction( dim1 % Direction_2D );
            diff.subtrahend_plane = dim2 / Direction_2D;
            diff.subtrahend_dir = Direction( dim2 % Direction_2D );

            float corr = std::abs( correlation( diff ) );
            DEBUG("Correlation of dim " << dim1 << " with " << dim2 << " is " << corr);
            if ( corr > max_corr ) {
                max_corr = corr;
                most_discriminating_diff = diff;
            }
        }
    DEBUG("Most discriminating dimension is " << most_discriminating_diff->minuend_plane << "," << most_discriminating_diff->minuend_dir << " against " << most_discriminating_diff->subtrahend_plane << "," << most_discriminating_diff->subtrahend_dir);
}

float InitialValueFinder::correlation( const SigmaDiff& sd ) const
{
    accumulator_set< double, stats< tag::variance, tag::covariance<double,tag::covariate1> > > diff_acc;
    accumulator_set< double, stats< tag::variance > > z_acc;

    const threed_info::ZPosition scan_step = 5E-8f * si::meter;
    const threed_info::SigmaDiffLookup lookup = sd.lookup(info);

    threed_info::ZRange range( lookup.get_z_range() );
    for ( threed_info::ZPosition z = lower( range ); z < upper( range ); z += scan_step )
    {
        diff_acc( lookup.get_sigma_diff(z).value(), covariate1 = z.value() );
        z_acc( z.value() );
    }

    double total_variance = variance(diff_acc) * variance( z_acc );
    if ( total_variance <= 1E-30 )
        return 0;
    else return covariance( diff_acc ) / sqrt( total_variance );
}

class InitialValueFinder::set_parameter {
    TraitValueFinder base;
    const InitialValueFinder& p;
    const guf::Spot& s;
    const PlaneEstimate& e;

  public:
    typedef void result_type;
    set_parameter( const InitialValueFinder& p, const guf::Spot& s, const PlaneEstimate& e, const dStorm::traits::Optics& o ) 
        : base( p.info.fluorophore, o ), p(p), s(s), e(e) {}

    template <typename Model>
    void operator()( nonlinfit::Xs<0,PSF::LengthUnit> p, Model& m ) {}
    template <typename Model>
    void operator()( nonlinfit::Xs<1,PSF::LengthUnit> p, Model& m ) {}
    template <int Dim, typename Model>
    void operator()( PSF::Mean<Dim> p, Model& m ) 
        { m( p ) = s[Dim]; }
    void operator()( PSF::MeanZ p, PSF::Polynomial3D& m ) 
        { m( p ) = e.z_estimate; }
    void operator()( PSF::MeanZ p, PSF::Spline3D& m ) { 
        m( p ) = e.z_estimate; 
    }
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

    if ( dynamic_cast<PSF::No3D*>(&position[0][0]) == NULL ) {
        estimate_z( data, e );
    }
    for (int p = 0; p < info.traits.plane_count(); ++p) {
        assert( ( position[p].kernel_count() ) == 1 );
        set_parameter s( *this, spot, e[p], info.traits.optics(p) );
        if ( PSF::Polynomial3D* z = dynamic_cast<PSF::Polynomial3D*>(&position[p][0]) ) {
            boost::mpl::for_each< PSF::Polynomial3D::Variables >( 
                boost::bind( boost::ref(s), _1, boost::ref( *z ) ) );
        } else if ( PSF::No3D* z = dynamic_cast<PSF::No3D*>(&position[p][0]) ) {
            boost::mpl::for_each< PSF::No3D::Variables >( 
                boost::bind( boost::ref(s), _1, boost::ref( *z ) ) );
        } else if ( PSF::Spline3D* z = dynamic_cast<PSF::Spline3D*>(&position[p][0]) ) {
            boost::mpl::for_each< PSF::Spline3D::Variables >( 
                boost::bind( boost::ref(s), _1, boost::ref( *z ) ) );
            z->set_spline( info.traits.optics(p).depth_info() );
        } else
            throw std::logic_error("Somebody forgot a 3D model in " + std::string(__FILE__) );
        s( constant_background::Amount(), position[p].background_model() );
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

void InitialValueFinder::estimate_z( const guf::Statistics<3>& s, std::vector<PlaneEstimate>& v ) const
{
    const SigmaDiff& mdm = *most_discriminating_diff;
    const threed_info::SigmaDiffLookup lookup = mdm.lookup(info);
    const threed_info::Sigma diff (
          s[ mdm.minuend_plane ].sigma[ mdm.minuend_dir ] 
        - s[ mdm.subtrahend_plane ].sigma[ mdm.subtrahend_dir ] );

    boost::optional<threed_info::ZPosition> z = lookup.look_up_sigma_diff( diff, 1E-8f * si::meter );
    if ( ! z ) z = info.traits.optics(0).depth_info()->equifocal_plane();
    DEBUG("Initial Z estimate with sigma-diff " << diff << " is " << *z);

    for (size_t i = 0; i < v.size(); ++i)
        v[i].z_estimate = *z;
}

std::vector<InitialValueFinder::PlaneEstimate> InitialValueFinder::estimate_bg_and_amp( 
    const guf::Spot&,
    const guf::Statistics<3> & s
) const {
    std::vector<PlaneEstimate> rv( s.size() );
    for (int i = 0; i < int(s.size()); ++i) {
        const traits::Optics& o = info.traits.optics(i);
        /* Value of perfectly sharp PSF at Z = 0 */
        double pif = o.transmission_coefficient(info.fluorophore);
            
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

    int highest_amp_plane = 0;
    for (int i = 1; i < int(s.size()); ++i) {
        if ( rv[i].amp > rv[highest_amp_plane].amp )
            highest_amp_plane = i;
    }

    for (int i = 0; i < int(s.size()); ++i) {
        rv[i].z_estimate = rv[highest_amp_plane].z_estimate;
    }

    return rv;
}

}
}
