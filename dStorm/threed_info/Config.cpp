#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include "Config.h"
#include <simparm/Object.hh>
#include <simparm/FileEntry.hh>
#include <simparm/Entry_Impl.hh>
#include <boost/smart_ptr/make_shared.hpp>
#include <dStorm/units/microlength.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/units/permicrolength.h>
#include <dStorm/threed_info/Spline3D.h>
#include <dStorm/threed_info/No3D.h>
#include <dStorm/threed_info/Polynomial3D.h>
#include <dStorm/polynomial_3d.h>

namespace dStorm {
namespace threed_info {

using namespace boost::units;

class No3DConfig : public simparm::Object, public Config {
    typedef  Eigen::Matrix< quantity< si::nanolength, double >, 2, 1, Eigen::DontAlign > PSFSize;
    simparm::Entry<PSFSize> psf_size;

    boost::shared_ptr<DepthInfo> make_traits( Direction dir ) const { 
        boost::shared_ptr<No3D> rv( new No3D() );
        rv->sigma = Sigma(psf_size()[dir] / 2.35);
        return rv;
    }
    void read_traits( const DepthInfo& dx, const DepthInfo& dy ) {
        PSFSize s;
        s[Direction_X] = PSFSize::Scalar( dynamic_cast<const No3D&>(dx).sigma * 2.35f );
        s[Direction_Y] = PSFSize::Scalar( dynamic_cast<const No3D&>(dy).sigma * 2.35f );
        psf_size = s;
    }
    void set_context() {}
    simparm::Node& getNode() { return *this; }
    void registerNamedEntries() { push_back( psf_size ); }
  public:
    No3DConfig() 
        : simparm::Object("No3D", "No 3D"),
          psf_size("PSF", "PSF FWHM", PSFSize::Constant(500.0 * boost::units::si::nanometre)) 
    { 
        psf_size.helpID = "PSF.FWHM";
        registerNamedEntries(); 
    }
    No3DConfig* clone() const { 
        No3DConfig* p = new No3DConfig(*this); 
        p->registerNamedEntries();
        return p;
    }
};

class Polynomial3DConfig : public simparm::Object, public Config {
    typedef  Eigen::Matrix< quantity< si::nanolength, double >, 2, 1, Eigen::DontAlign > PSFSize;
    simparm::Entry<PSFSize> psf_size;
    typedef Eigen::Matrix< boost::units::quantity<boost::units::si::nanolength, double>, 2, 1, Eigen::DontAlign > ZPosition;
    simparm::Entry< ZPosition > z_position, z_range;
    typedef simparm::Entry< 
        Eigen::Matrix< quantity<si::permicrolength>, Direction_2D, 
                       polynomial_3d::Order, Eigen::DontAlign > > SlopeEntry;
    SlopeEntry slopes;

    boost::shared_ptr<DepthInfo> make_traits( Direction dir ) const;
    void read_traits( const DepthInfo&, const DepthInfo& );
    void set_context() { }
    simparm::Node& getNode() { return *this; }
    void registerNamedEntries() { push_back( psf_size ); push_back( z_position ); push_back( slopes ); push_back( z_range ); }
  public:
    Polynomial3DConfig();
    Polynomial3DConfig* clone() const { 
        Polynomial3DConfig* p = new Polynomial3DConfig(*this); 
        p->registerNamedEntries();
        return p;
    }
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
: simparm::Object("Polynomial3D", "Polynomial 3D"),
  psf_size("PSF", "PSF FWHM at sharpest Z", PSFSize::Constant(500.0 * boost::units::si::nanometre)),
  z_position("ZPosition", "Point of sharpest Z", ZPosition::Constant(0 * si::nanometre)),
  z_range("ZRange", "Maximum sensible Z distance from equifocused plane", ZPosition::Constant(1000 * boost::units::si::nanometre)),
  slopes("WideningConstants", "Widening slopes")
{
    slopes.helpID = "Polynomial3D.WideningSlopes";
    psf_size.helpID = "PSF.FWHM";
    z_position.setHelp("Z position where this layer is sharpest in this dimension");
    registerNamedEntries();
}

class Spline3DConfig : public simparm::Object, public Config {
    simparm::FileEntry z_calibration_file;

    boost::shared_ptr<DepthInfo> make_traits( Direction dir ) const {
        if ( z_calibration_file )
            return boost::shared_ptr<DepthInfo>(new Spline3D( SplineFactory( z_calibration_file(), dir) ));
        else
            return boost::shared_ptr<DepthInfo>();
    }
    void read_traits( const DepthInfo&, const DepthInfo& ) 
        { z_calibration_file = ""; }
    void set_context() {}
    simparm::Node& getNode() { return *this; }
    void registerNamedEntries() {}
  public:
    Spline3DConfig() 
        : simparm::Object("Spline3D", "Interpolated 3D"),
          z_calibration_file("ZCalibration", "Z calibration file") { registerNamedEntries(); }
    Spline3DConfig* clone() const { 
        Spline3DConfig* p = new Spline3DConfig(*this); 
        p->registerNamedEntries();
        return p;
    }
};


std::auto_ptr< Config > make_no_3d_config() 
    { return std::auto_ptr< Config >( new No3DConfig() ); }
std::auto_ptr< Config > make_polynomial_3d_config()
    { return std::auto_ptr< Config >( new Polynomial3DConfig() ); }
std::auto_ptr< Config > make_spline_3d_config()
    { return std::auto_ptr< Config >( new Spline3DConfig() ); }

}
}
