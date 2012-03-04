#ifndef DSTORM_TRAITS_RESOLUTION_CONFIG_H
#define DSTORM_TRAITS_RESOLUTION_CONFIG_H

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include <simparm/BoostOptional.hh>
#include <simparm/VectorEntry.hh>
#include <boost/units/power10.hpp>
#include <dStorm/UnitEntries/PixelSize.h>
#include <dStorm/localization/Traits.h>
#include <simparm/ChoiceEntry.hh>
#include <dStorm/image/fwd.h>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/traits/optics_config.h>
#include <dStorm/Direction.h>
#include <dStorm/units/microlength.h>

namespace dStorm {
namespace traits {
namespace resolution {

typedef power_typeof_helper< 
        power10< si::length, -6 >::type,
        static_rational<-1> >::type PerMicro; 

struct ThreeDConfig {
    virtual ~ThreeDConfig() {}
    virtual DepthInfo set_traits() const = 0;
    virtual void read_traits( const DepthInfo& ) = 0;
    virtual simparm::Node& getNode() = 0;
    operator simparm::Node&() { return getNode(); }
    operator const simparm::Node&() const { return const_cast<ThreeDConfig&>(*this).getNode(); }
    virtual ThreeDConfig* clone() const = 0;
};

class NoThreeDConfig : public simparm::Object, public ThreeDConfig {
    DepthInfo set_traits() const;
    void read_traits( const DepthInfo& );
    simparm::Node& getNode() { return *this; }
  public:
    NoThreeDConfig() : simparm::Object("No3D", "No 3D") {}
    NoThreeDConfig* clone() const { return new NoThreeDConfig(); }
};

class Polynomial3DConfig : public simparm::Object, public ThreeDConfig {
    typedef simparm::vector_entry< quantity< si::microlength >, Direction_2D >::type
        FocalDepthEntry;
    typedef simparm::matrix_entry
        < double, Direction_2D, Polynomial3D::Order >::type PrefactorEntry;
    FocalDepthEntry focal_depth;
    PrefactorEntry prefactors;

    DepthInfo set_traits() const;
    void read_traits( const DepthInfo& );
    simparm::Node& getNode() { return *this; }
    void registerNamedEntries() { push_back( focal_depth ); push_back( prefactors); }
  public:
    Polynomial3DConfig();
    Polynomial3DConfig(const Polynomial3DConfig&);
    Polynomial3DConfig* clone() const { return new Polynomial3DConfig(*this); }
};

class Config : public simparm::Object {
    simparm::NodeChoiceEntry< ThreeDConfig > three_d;
    traits::CuboidConfig cuboid_config;

  public:
    static traits::ImageResolution
        get( const FloatPixelSizeEntry::value_type& f );

    Config();
    ~Config();
    void registerNamedEntries();
    void set_traits( input::Traits<Localization>& ) const;
    void set_traits( engine::InputTraits& ) const;
    void read_traits( const engine::InputTraits& );
    void read_traits( const input::Traits<Localization>& );
    image::MetaInfo<2>::Resolutions get_resolution() const;
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
