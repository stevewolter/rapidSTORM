#include <simparm/Eigen_decl.h>
#include <simparm/BoostUnits.h>
#include <simparm/Eigen.h>
#include <simparm/Object.h>
#include <simparm/Entry.h>

#include <boost/units/Eigen/Core>
#include <boost/units/Eigen/Array>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>

#include "LengthUnit.h"
#include "units/nanolength.h"
#include "units/microlength.h"
#include "units/permicrolength.h"

#include "threed_info/Config.h"
#include "threed_info/DepthInfo.h"
#include "threed_info/Polynomial3D.h"

namespace dStorm {
namespace threed_info {

using namespace boost::units;

Polynomial3D::Polynomial3D() 
: sigma_(0.0),
  z_position( 0.0 ),
  z_limit_( 0.0 ),
  widening( Widening::Constant( 0.0 ) )
{
}

double Polynomial3D::get_slope( int term ) const
{
    assert( term >= MinTerm && term <= Order );
    return widening[ term ];
}

void Polynomial3D::set_slope( int term, double s )
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

bool Polynomial3D::is_positive_over_depth_range() const {
    double a = pow(1.0/get_slope(4), 4.0);
    double b = pow(1.0/get_slope(3), 3.0);
    double c = pow(1.0/get_slope(2), 2.0);
    double d = 1.0/get_slope(1);
    double e = 1;

    // According to en.wikipedia.org/wiki/Discriminant
    double discriminant =
        256*pow(a,3)*pow(e,3) - 192*pow(a,2)*b*d*pow(e,2)
        -128*pow(a,2)*pow(c,2)*pow(e,2) + 144*pow(a,2)*c*pow(d,2)*e
        -27*pow(a,2)*pow(d,4) + 144*a*pow(b,2)*c*pow(e,2)
        -6*a*pow(b,2)*pow(d,2)*e - 80*a*b*pow(c,2)*d*e
        +18*a*b*c*pow(d,3) + 16*a*pow(c,4)*e
        -4*a*pow(c,3)*pow(d,2) - 27*pow(b,4)*pow(e,2)
        + 18*pow(b,3)*c*d*e - 4*pow(b,3)*pow(d,3) - 4*pow(b,2)*pow(c,3)*e
        +pow(b,2)*pow(c,2)*pow(d,2);
    // According to https://en.wikipedia.org/wiki/Quartic_function#Solving_a_quartic_equation
    double p = 8 * a * c - 3 * pow(b,2);
    double D = 64 * pow(a,3) * e - 16 * a * a * c * c + 16 * a * b * b * c - 16 * a * a * b * d - 3 * pow(b,4);

    // We want a function with only complex roots. The fourth-order and second-order terms should dominate.
    return (discriminant > 0 && (p > 0 || D > 0));
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

template <typename ToQuantity>
Eigen::Matrix< ToQuantity, 2, 1 >
matrify( double x, double y )
{
    Eigen::Matrix< ToQuantity, 2, 1 > rv;
    rv.x() = ToQuantity(FromLengthUnit(x));
    rv.y() = ToQuantity(FromLengthUnit(y));
    return rv;
}

void Polynomial3DConfig::read_traits( const DepthInfo& dx, const DepthInfo& dy )
{
    const Polynomial3D& px = static_cast< const Polynomial3D& >(dx);
    const Polynomial3D& py = static_cast< const Polynomial3D& >(dy);
    z_range = matrify<ZPosition::Scalar>(px.z_limit(), py.z_limit());
    z_position = matrify<ZPosition::Scalar>(px.focal_plane(), py.focal_plane());

    psf_size = matrify<PSFSize::Scalar>( px.get_base_width() * 2.35f, py.get_base_width() * 2.35f );

    SlopeEntry::value_type slopes;
    for ( Direction dir = Direction_First; dir < Direction_2D; ++dir ) {
        const Polynomial3D& p = (dir == Direction_X) ? px : py;
        for ( int term = Polynomial3D::MinTerm; term <= Polynomial3D::Order; ++term ) {
            slopes(dir, term-Polynomial3D::MinTerm) = 
                SlopeEntry::value_type::Scalar(1.0 / FromLengthUnit(p.get_slope(term)));
        }
    }
    this->slopes = slopes;
}

boost::shared_ptr<DepthInfo> Polynomial3DConfig::make_traits(Direction dir) const {
    boost::shared_ptr<Polynomial3D> p( new Polynomial3D() );
    p->set_z_limit( ToLengthUnit(z_range()[dir]) );
    p->set_focal_plane( ToLengthUnit(z_position()[dir]) );
    p->set_base_width( ToLengthUnit(psf_size()[dir] / 2.35) );
    for ( int term = Polynomial3D::MinTerm; term <= Polynomial3D::Order; ++term ) {
        quantity< si::permicrolength > s = slopes()( dir, term-Polynomial3D::MinTerm );
        if ( abs(s) < 1E-30 / si::micrometer )
            p->set_slope( term, ToLengthUnit(1E24 * si::meter) );
        else
            p->set_slope( term, ToLengthUnit(1.0 / s) ) ;
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
