#include "debug.h"
#include <Eigen/StdVector>
#include "engine/JobInfo.h"
#include <boost/iterator/zip_iterator.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include "guf/InitialValueFinder.h"
#include "gaussian_psf/expressions.h"
#include "constant_background/model.hpp"
#include "guf/TraitValueFinder.h"
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include "threed_info/look_up_sigma_diff.h"
#include <boost/accumulators/statistics/covariance.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/variates/covariate.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/framework/accumulator_set.hpp>
#include "threed_info/look_up_sigma_diff.h"

using namespace boost::accumulators;

namespace dStorm {
namespace guf {

struct InitialValueFinder::PlaneEstimate { 
    double bg; 
    double amp; 
    boost::units::quantity<boost::units::si::length> z_estimate; 
};

threed_info::SigmaDiffLookup InitialValueFinder::SigmaDiff::lookup( const engine::InputTraits& info ) const {
    return threed_info::SigmaDiffLookup(
        *info.optics( minuend_plane ).depth_info( minuend_dir ),
        *info.optics( subtrahend_plane ).depth_info( subtrahend_dir ),
        1E-2 );
}

bool InitialValueFinder::determine_z_estimate_need( const engine::InputTraits& t ) {
    for ( engine::InputTraits::const_iterator i = t.begin(); i != t.end(); ++i )
        for ( Direction dir = Direction_First; dir != Direction_2D; ++dir )
            if ( i->optics.depth_info(dir)->provides_3d_info() )
                return true;
    return false;
}

void InitialValueFinder::create_z_lookup_table( const engine::InputTraits& t )
{
    float max_corr = -1;
    const int dim_count = Direction_2D * t.plane_count();
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
    lookup_table.reset( new threed_info::SigmaDiffLookup( most_discriminating_diff->lookup( t ) ) );
}

InitialValueFinder::InitialValueFinder( const Config& config, const dStorm::engine::JobInfo& info) 
: info(info),
  disjoint_amplitudes( config.disjoint_amplitudes() ),
  need_z_estimate( determine_z_estimate_need(info.traits) )
{
    if ( need_z_estimate ) create_z_lookup_table( info.traits );
}

InitialValueFinder::~InitialValueFinder() {}

float InitialValueFinder::correlation( const SigmaDiff& sd ) const
{
    accumulator_set< double, stats< tag::variance, tag::covariance<double,tag::covariate1> > > diff_acc;
    accumulator_set< double, stats< tag::variance > > z_acc;

    const threed_info::ZPosition scan_step = 5E-2;
    const threed_info::SigmaDiffLookup lookup = sd.lookup(info.traits);

    threed_info::ZRange range( lookup.get_z_range() );
    for ( threed_info::ZPosition z = lower( range ); z < upper( range ); z += scan_step )
    {
        diff_acc( lookup.get_sigma_diff(z), covariate1 = z );
        z_acc( z );
    }

    double total_variance = variance(diff_acc) * variance( z_acc );
    if ( total_variance <= 1E-30 )
        return 0;
    else 
        return covariance( diff_acc ) / sqrt( total_variance );
}

class InitialValueFinder::set_parameter {
    TraitValueFinder base;
    const InitialValueFinder& p;
    const Spot& s;
    const PlaneEstimate& e;

  public:
    typedef void result_type;
    set_parameter( const InitialValueFinder& p, const Spot& s, const PlaneEstimate& e, const dStorm::traits::Optics& o ) 
        : base( p.info.fluorophore, o ), p(p), s(s), e(e) {}

    template <typename Model>
    void operator()( nonlinfit::Xs<0> p, Model& m ) {}
    template <typename Model>
    void operator()( nonlinfit::Xs<1> p, Model& m ) {}
    template <int Dim, typename Model>
    void operator()( gaussian_psf::Mean<Dim> p, Model& m ) 
        { m( p ) = s[Dim]; }
    void operator()( gaussian_psf::MeanZ p, gaussian_psf::DepthInfo3D& m ) { 
        m( p ) = quantity<si::length>(e.z_estimate).value() * 1E6; 
    }
    template <typename Model>
    void operator()( gaussian_psf::Amplitude a, Model& m ) 
        { m( a ) = e.amp; }
    void operator()( constant_background::Amount a, constant_background::Expression& m ) 
        { m( a ) = e.bg; }
    template <typename Parameter, typename Model>
    void operator()( Parameter p, Model& m ) { base( p, m ); }
};

void InitialValueFinder::operator()( 
    MultiKernelModelStack& position, 
    const Spot& spot,
    const fit_window::PlaneStack& data
) const {
    std::vector<PlaneEstimate> e = estimate_bg_and_amp(spot,data);
    if ( ! disjoint_amplitudes ) join_amp_estimates( e );

    if ( need_z_estimate ) {
        estimate_z( data, e );
    }
    for (int p = 0; p < info.traits.plane_count(); ++p) {
        assert( ( position[p].kernel_count() ) == 1 );
        set_parameter s( *this, spot, e[p], info.traits.optics(p) );
        if ( gaussian_psf::No3D* z = dynamic_cast<gaussian_psf::No3D*>(&position[p][0]) ) {
            boost::mpl::for_each< gaussian_psf::No3D::Variables >( 
                boost::bind( boost::ref(s), _1, boost::ref( *z ) ) );
        } else if ( gaussian_psf::DepthInfo3D* z = dynamic_cast<gaussian_psf::DepthInfo3D*>(&position[p][0]) ) {
            boost::mpl::for_each< gaussian_psf::DepthInfo3D::Variables >( 
                boost::bind( boost::ref(s), _1, boost::ref( *z ) ) );
            z->set_spline( 
                info.traits.optics(p).depth_info(Direction_X),
                info.traits.optics(p).depth_info(Direction_Y) );
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

void InitialValueFinder::estimate_z( const fit_window::PlaneStack& s, std::vector<PlaneEstimate>& v ) const
{
    const SigmaDiff& mdm = *most_discriminating_diff;
    boost::optional<threed_info::ZPosition> z = (*lookup_table)( 
        s[ mdm.minuend_plane ].standard_deviation[ mdm.minuend_dir ],
        s[ mdm.subtrahend_plane ].standard_deviation[ mdm.subtrahend_dir ] );

    for (size_t i = 0; i < v.size(); ++i)
        v[i].z_estimate = *z * 1E-6 * si::meter;
}

std::vector<InitialValueFinder::PlaneEstimate> InitialValueFinder::estimate_bg_and_amp( 
    const Spot&,
    const fit_window::PlaneStack & s
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
            rv[i].bg = s[i].background_estimate;
            rv[i].amp = std::max( 
                (s[i].integral - rv[i].bg * double(s[i].points.size())) / pif,
                1.0 );
        } else {
            rv[i].amp = 0;
            rv[i].bg = s[i].integral / s[i].points.size();
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
