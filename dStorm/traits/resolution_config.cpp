#include "debug.h"
#include "resolution_config.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include <boost/variant/get.hpp>

namespace dStorm {
namespace traits {
namespace resolution {

Config::Config()
: simparm::Object("Optics", "Optical parameters"),
  three_d("ThreeD", "3D PSF model")
{
    three_d.helpID = "3DType";
    three_d.addChoice( new NoThreeDConfig() );
    three_d.addChoice( new Polynomial3DConfig() );
    cuboid_config.set_3d_availability(false);
}

Config::~Config() {}

void Config::registerNamedEntries() {
    push_back( three_d );
    cuboid_config.registerNamedEntries();
    push_back( cuboid_config );
}

Polynomial3DConfig::Polynomial3DConfig()
: simparm::Object("Polynomial3D", "Polynomial 3D"),
  slopes("WideningConstants", "Widening slopes")
{
    slopes.helpID = "Polynomial3D.WideningSlopes";

    registerNamedEntries();
}

Polynomial3DConfig::Polynomial3DConfig(const Polynomial3DConfig& o)
: simparm::Object(o), slopes(o.slopes)
{
    registerNamedEntries();
}

void Config::write_traits( engine::InputTraits& t ) const
{
    DEBUG("Setting traits in ResolutionSetter, input is " << t.size.transpose());
    cuboid_config.write_traits(t);

    t.depth_info = three_d().set_traits();
    const_cast< traits::CuboidConfig& >(cuboid_config).set_3d_availability( 
        boost::get<traits::No3D>(t.depth_info.get_ptr()) == NULL );
}

DepthInfo NoThreeDConfig::set_traits() const { return traits::No3D(); }

void NoThreeDConfig::read_traits( const DepthInfo& d )
{
    assert( boost::get< traits::No3D >( &d ) );
}

void Polynomial3DConfig::read_traits( const DepthInfo& d )
{
    const Polynomial3D& p = boost::get< traits::Polynomial3D >(d);
    SlopeEntry::value_type slopes;

    for ( Direction dir = Direction_First; dir < Direction_2D; ++dir ) {
        for ( int term = Polynomial3D::MinTerm; term <= Polynomial3D::Order; ++term ) {
            slopes(dir, term-Polynomial3D::MinTerm) = 
                SlopeEntry::value_type::Scalar(1.0 / p.get_slope(dir,term));
        }
    }
    this->slopes = slopes;
}

DepthInfo Polynomial3DConfig::set_traits() const {
    traits::Polynomial3D p;
    for ( Direction dir = Direction_First; dir < Direction_2D; ++dir ) {
        for ( int term = Polynomial3D::MinTerm; term <= Polynomial3D::Order; ++term ) {
            quantity< PerMicro > s = slopes()( dir, term-Polynomial3D::MinTerm );
            if ( s < 1E-30 / si::micrometer )
                p.set_slope( dir, term, 1E24 * si::meter );
            else
                p.set_slope(dir, term, traits::Polynomial3D::WidthSlope( pow<-1>(s) ) ) ;
        }
    }
    return p;
}

traits::ImageResolution
Config::get( const FloatPixelSizeEntry::value_type& f ) {
    boost::units::quantity< boost::units::divide_typeof_helper<
        boost::units::si::length,camera::length>::type, float > q1;
    q1 = (f / (1E9 * boost::units::si::nanometre) * boost::units::si::metre);
    return q1;
}

void Config::write_traits( input::Traits<Localization>& t ) const {
    DEBUG("Setting resolution in Config");
    t.position().resolution() = cuboid_config.make_localization_traits();
}

void Config::read_plane_count( const engine::InputTraits& t ) {
    cuboid_config.set_number_of_planes( t.plane_count() );
    cuboid_config.set_number_of_fluorophores( t.fluorophores.size() );
}
void Config::read_plane_count( const input::Traits<Localization>& t ) {
    cuboid_config.set_number_of_planes( 1 );
    cuboid_config.set_number_of_fluorophores( t.fluorophores.size() );
}

void Config::read_traits( const engine::InputTraits& t ) {
    cuboid_config.read_traits( t );
    if ( t.depth_info.is_initialized() ) {
        if ( boost::get< traits::No3D >( t.depth_info.get_ptr() ) ) {
            three_d.choose( "No3D" );
            cuboid_config.set_3d_availability(false);
        } else if ( boost::get< traits::Polynomial3D >( t.depth_info.get_ptr() ) ) {
            three_d.choose( "Polynomial3D" );
            cuboid_config.set_3d_availability(true);
        } else
            throw std::runtime_error("ThreeD info not recognized");
        three_d().read_traits( *t.depth_info );
    }
}

void Config::read_traits( const input::Traits<dStorm::Localization>& ) {
    /* TODO: Implementation of this function would enable displaying resolutions
     * from STM files. */
}

image::MetaInfo<2>::Resolutions Config::get_resolution() const
{
    return cuboid_config.get_resolution();
}

}
}
}

namespace boost {
namespace units {

std::string name_string(const dStorm::traits::resolution::PerMicro&)
{
    return "micrometer^-1";
}

std::string symbol_string(const dStorm::traits::resolution::PerMicro&)
{
    return "µm^-1";
}

}
}
