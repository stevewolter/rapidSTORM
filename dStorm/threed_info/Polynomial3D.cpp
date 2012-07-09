#include <simparm/Eigen_decl.h>
#include <simparm/BoostUnits.h>
#include <simparm/Eigen.h>
#include <simparm/Object.h>
#include <simparm/Entry.h>

#include <boost/units/Eigen/Core>
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>

#include <dStorm/units/nanolength.h>
#include <dStorm/units/microlength.h>
#include <dStorm/units/permicrolength.h>

#include "Config.h"
#include "DepthInfo.h"
#include "Polynomial3D.h"

namespace dStorm {
namespace threed_info {

using namespace boost::units;

Polynomial3D::Polynomial3D() 
: sigma_(0.0 * si::meter),
  z_position( 0.0 * si::meter ),
  z_limit_( 0.0 * si::meter ),
  widening( Widening::Constant( 0.0 * si::meter ) )
{
}

Polynomial3D::WidthSlope Polynomial3D::get_slope( int term ) const
{
    assert( term >= MinTerm && term <= Order );
    return widening[ term ];
}

void Polynomial3D::set_slope( int term, WidthSlope s )
{
    assert( term >= MinTerm && term <= Order );
    widening[ term ] = s;
}

Sigma Polynomial3D::get_sigma_( ZPosition z ) const 
{
    return sigma_ * float(sqrt(sigma_scaling_factor(z)));
}

double Polynomial3D::sigma_scaling_factor( ZPosition z ) const {
    ZPosition z_offset = (z - z_position);
    double prefactor = 1;
    for (int i = MinTerm; i <= Order; ++i ) {
        prefactor += pow( z_offset / widening[ i ], i );
    }
    return prefactor;
}

SigmaDerivative Polynomial3D::get_sigma_deriv_( ZPosition z ) const {
    ZPosition z_offset = (z - z_position);
    double prefactor_deriv = 0;
    for (int i = MinTerm; i <= Order; ++i ) {
        prefactor_deriv += 
                pow( z_offset / widening[ i ], i-1 ) * (sigma_ / widening[ i ] );
    }
    return (prefactor_deriv / (2 * sqrt( sigma_scaling_factor(z) )));
}

ZRange Polynomial3D::z_range_() const {
    ZRange rv;
    rv += ZInterval( z_position - z_limit_, z_position + z_limit_ );
    return rv;
}

std::ostream& Polynomial3D::print_( std::ostream& o ) const {
    o << "polynomial 3D with best-focused FWHM " << sigma_ 
      << "and focus depths " ;
    for (int j = threed_info::Polynomial3D::MinTerm; j <= threed_info::Polynomial3D::Order; ++j)
        o << 1.0 / get_slope(j) << " ";
    return o << " and focal plane " << z_position;
}

class Polynomial3DConfig : public Config {
    typedef  Eigen::Matrix< quantity< si::nanolength, double >, 2, 1, Eigen::DontAlign > PSFSize;
    simparm::Entry<PSFSize> psf_size;
    typedef Eigen::Matrix< boost::units::quantity<boost::units::si::nanolength, double>, 2, 1, Eigen::DontAlign > ZPosition;
    simparm::Entry< ZPosition > z_position, z_range;
    typedef Eigen::Matrix< quantity<si::permicrolength>, Direction_2D, 
                       polynomial_3d::Order, Eigen::DontAlign > Slopes;
    typedef simparm::Entry< Slopes > SlopeEntry;
    SlopeEntry slopes;

    boost::shared_ptr<DepthInfo> make_traits( Direction dir ) const;
    void read_traits( const DepthInfo&, const DepthInfo& );
    void set_context() { }
    void attach_ui( simparm::NodeHandle to ) { 
        simparm::NodeHandle r = attach_parent(to);
        psf_size.attach_ui( r );
        z_position.attach_ui( r );
        slopes.attach_ui( r ); 
        z_range.attach_ui( r ); 
    }
  public:
    Polynomial3DConfig();
    Polynomial3DConfig* clone() const { return new Polynomial3DConfig(*this); }
};

template <typename ToQuantity, typename FromUnit, typename Base>
Eigen::Matrix< ToQuantity, 2, 1 >
matrify( quantity<FromUnit,Base> x, quantity<FromUnit,Base> y )
{
    Eigen::Matrix< ToQuantity, 2, 1 > rv;
    rv.x() = ToQuantity(x);
    rv.y() = ToQuantity(y);
    return rv;
}

void Polynomial3DConfig::read_traits( const DepthInfo& dx, const DepthInfo& dy )
{
    const Polynomial3D& px = static_cast< const Polynomial3D& >(dx);
    const Polynomial3D& py = static_cast< const Polynomial3D& >(dy);
    z_range = matrify<ZPosition::Scalar>(px.z_limit(), py.z_limit());
    z_position = matrify<ZPosition::Scalar>(px.focal_plane(), py.focal_plane());

    psf_size = matrify< PSFSize::Scalar >( px.get_base_width() * 2.35f, py.get_base_width() * 2.35f );

    SlopeEntry::value_type slopes;
    for ( Direction dir = Direction_First; dir < Direction_2D; ++dir ) {
        const Polynomial3D& p = (dir == Direction_X) ? px : py;
        for ( int term = Polynomial3D::MinTerm; term <= Polynomial3D::Order; ++term ) {
            slopes(dir, term-Polynomial3D::MinTerm) = 
                SlopeEntry::value_type::Scalar(1.0 / p.get_slope(term));
        }
    }
    this->slopes = slopes;
}

boost::shared_ptr<DepthInfo> Polynomial3DConfig::make_traits(Direction dir) const {
    boost::shared_ptr<Polynomial3D> p( new Polynomial3D() );
    p->set_z_limit( threed_info::ZPosition(z_range()[dir]) );
    p->set_focal_plane( threed_info::ZPosition(z_position()[dir]) );
    p->set_base_width( Sigma(psf_size()[dir] / 2.35) );
    for ( int term = Polynomial3D::MinTerm; term <= Polynomial3D::Order; ++term ) {
        quantity< si::permicrolength > s = slopes()( dir, term-Polynomial3D::MinTerm );
        if ( s < 1E-30 / si::micrometer )
            p->set_slope( term, 1E24 * si::meter );
        else
            p->set_slope( term, Polynomial3D::WidthSlope( pow<-1>(s) ) ) ;
    }
    return p;
}


Polynomial3DConfig::Polynomial3DConfig()
: Config("Polynomial3D"),
  psf_size("SharpestPSF", PSFSize::Constant(500.0 * boost::units::si::nanometre)),
  z_position("ZPosition", ZPosition::Constant(0 * si::nanometre)),
  z_range("ZRange", ZPosition::Constant(1000 * boost::units::si::nanometre)),
  slopes("WideningConstants", Slopes::Constant( quantity<si::permicrolength>(0.0 / si::metre) ) )
{
}

std::auto_ptr< Config > make_polynomial_3d_config()
    { return std::auto_ptr< Config >( new Polynomial3DConfig() ); }

}
}
