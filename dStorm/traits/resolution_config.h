#ifndef DSTORM_TRAITS_RESOLUTION_CONFIG_H
#define DSTORM_TRAITS_RESOLUTION_CONFIG_H

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include <simparm/BoostOptional.hh>
#include <boost/units/power10.hpp>
#include <dStorm/UnitEntries/PixelSize.h>
#include <dStorm/localization/Traits.h>
#include <simparm/ChoiceEntry.hh>
#include <dStorm/Image_decl.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/traits/optics_config.h>

namespace dStorm {
namespace traits {
namespace resolution {

typedef power_typeof_helper< 
        power10< si::length, -6 >::type,
        static_rational<-1> >::type PerMicro; 

struct ThreeDConfig {
    virtual ~ThreeDConfig() {}
    virtual void set_traits( input::Traits<engine::Image>& ) const = 0;
    virtual void read_traits( const dStorm::traits::Optics<3>::DepthInfo& ) = 0;
    virtual simparm::Node& getNode() = 0;
    operator simparm::Node&() { return getNode(); }
    operator const simparm::Node&() const { return const_cast<ThreeDConfig&>(*this).getNode(); }
    virtual ThreeDConfig* clone() const = 0;
};

class NoThreeDConfig : public simparm::Object, public ThreeDConfig {
    void set_traits( input::Traits<engine::Image>& ) const;
    void read_traits( const dStorm::traits::Optics<3>::DepthInfo& );
    simparm::Node& getNode() { return *this; }
  public:
    NoThreeDConfig() : simparm::Object("No3D", "No 3D") {}
    NoThreeDConfig* clone() const { return new NoThreeDConfig(); }
};

class ZhuangThreeDConfig : public simparm::Object, public ThreeDConfig {
    simparm::Entry< Eigen::Matrix< quantity< PerMicro, float >, 2, 1, Eigen::DontAlign > > widening;

    void set_traits( input::Traits<engine::Image>& ) const;
    void read_traits( const dStorm::traits::Optics<3>::DepthInfo& );
    simparm::Node& getNode() { return *this; }
    void registerNamedEntries() { push_back( widening ); }
  public:
    ZhuangThreeDConfig();
    ZhuangThreeDConfig(const ZhuangThreeDConfig&);
    ZhuangThreeDConfig* clone() const { return new ZhuangThreeDConfig(*this); }
};

class Config : public simparm::Object {
    typedef  Eigen::Matrix< quantity< si::nanolength, double >, 2, 1, Eigen::DontAlign > PSFSize;
    simparm::Entry<PSFSize> psf_size;
    simparm::NodeChoiceEntry< ThreeDConfig > three_d;
    traits::CuboidConfig cuboid_config;

  public:
    static traits::ImageResolution
        get( const FloatPixelSizeEntry::value_type& f );

    Config();
    ~Config();
    void registerNamedEntries();
    void set_traits( input::Traits<Localization>& ) const;
    void set_traits( input::Traits<engine::Image>& ) const;
    void read_traits( const input::Traits<engine::Image>& );
    void read_traits( const input::Traits<Localization>& );
    traits::Optics<2>::Resolutions get_resolution() const;
};


}
}
}

namespace boost {
namespace units {

std::string name_string(const dStorm::traits::resolution::PerMicro&);
std::string symbol_string(const dStorm::traits::resolution::PerMicro&);

}
}

#endif
