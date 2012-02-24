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
    three_d.addChoice( new ZhuangThreeDConfig() );
    cuboid_config.set_3d_availability(false);
}

Config::~Config() {}

void Config::registerNamedEntries() {
    push_back( three_d );
    cuboid_config.registerNamedEntries();
    push_back( cuboid_config );
}

ZhuangThreeDConfig::ZhuangThreeDConfig()
: simparm::Object("Zhuang3D", "Parabolic 3D"),
  widening("DefocusConstant", "Speed of PSF std. dev. growth")
{
    widening.helpID = "zhuang.DefocusConstant";
    registerNamedEntries();
}

ZhuangThreeDConfig::ZhuangThreeDConfig(const ZhuangThreeDConfig& o)
: simparm::Object(o), widening(o.widening)
{
    registerNamedEntries();
}

void Config::set_traits( engine::InputTraits& t ) const
{
    DEBUG("Setting traits in ResolutionSetter, input is " << t.size.transpose());
    cuboid_config.set_traits(t);

    t.depth_info = three_d().set_traits();
    const_cast< traits::CuboidConfig& >(cuboid_config).set_3d_availability( 
        boost::get<traits::No3D>(t.depth_info.get_ptr()) == NULL );
}

DepthInfo NoThreeDConfig::set_traits() const { return traits::No3D(); }

void NoThreeDConfig::read_traits( const DepthInfo& d )
{
    assert( boost::get< traits::No3D >( &d ) );
}

void ZhuangThreeDConfig::read_traits( const DepthInfo& d )
{
    const Zhuang3D::Widening& w = boost::get< traits::Zhuang3D >(d).widening;
    widening = w.cast< quantity<PerMicro,float> >();
}

DepthInfo ZhuangThreeDConfig::set_traits() const {
    traits::Zhuang3D zhuang;
    for (int i = 0; i < 2; ++i)
        zhuang.widening[i] = quantity<traits::Zhuang3D::Unit>( widening()[i] );
    return zhuang;
}

traits::ImageResolution
Config::get( const FloatPixelSizeEntry::value_type& f ) {
    boost::units::quantity< boost::units::divide_typeof_helper<
        boost::units::si::length,camera::length>::type, float > q1;
    q1 = (f / (1E9 * boost::units::si::nanometre) * boost::units::si::metre);
    return q1;
}

void Config::set_traits( input::Traits<Localization>& t ) const {
    DEBUG("Setting resolution in Config");
    t.position().resolution() = cuboid_config.make_localization_traits();
}

void Config::read_traits( const engine::InputTraits& t ) {
    cuboid_config.set_entries_to_traits( t, t.fluorophores.size() );
    if ( t.depth_info.is_initialized() ) {
        if ( boost::get< traits::No3D >( t.depth_info.get_ptr() ) )
            three_d.choose( "No3D" );
        else if ( boost::get< traits::Zhuang3D >( t.depth_info.get_ptr() ) )
            three_d.choose( "Zhuang3D" );
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
