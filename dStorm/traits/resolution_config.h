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
#include <dStorm/image/fwd.h>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/traits/optics_config.h>
#include <dStorm/Direction.h>
#include <dStorm/units/microlength.h>

namespace dStorm {
namespace traits {
namespace resolution {

class Config : public simparm::Object {
    traits::CuboidConfig cuboid_config;

  public:
    static traits::ImageResolution
        get( const FloatPixelSizeEntry::value_type& f );

    Config();
    ~Config();
    void registerNamedEntries();
    void write_traits( input::Traits<Localization>& ) const;
    void write_traits( engine::InputTraits& ) const;
    void read_traits( const engine::InputTraits& );
    void read_traits( const input::Traits<Localization>& );
    void set_context( const engine::InputTraits& );
    void set_context( const input::Traits<Localization>& );
    image::MetaInfo<2>::Resolutions get_resolution() const;
};

}
}
}

#endif
