#ifndef DSTORM_INPUT_RESOLUTIONSETTER_H
#define DSTORM_INPUT_RESOLUTIONSETTER_H

#include "debug.h"
#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/BoostOptional.hh>
#include <dStorm/input/Source.h>

#include <boost/units/power10.hpp>
#include <simparm/TreeCallback.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/UnitEntries/PixelSize.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/localization/Traits.h>
#include <simparm/Structure.hh>
#include <simparm/ChoiceEntry.hh>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Filter.h>
#include <dStorm/Image_decl.h>
#include <dStorm/ImageTraits.h>

#include <dStorm/traits/optics_config.h>

namespace dStorm {
namespace input {

using namespace chain;

namespace Resolution {

typedef power_typeof_helper< 
        power10< si::length, -6 >::type,
        static_rational<-1> >::type PerMicro; 

struct ThreeDConfig {
    virtual ~ThreeDConfig() {}
    virtual void set_traits( input::Traits<engine::Image>& ) const = 0;
    virtual simparm::Node& getNode() = 0;
    operator simparm::Node&() { return getNode(); }
    operator const simparm::Node&() const { return const_cast<ThreeDConfig&>(*this).getNode(); }
    virtual ThreeDConfig* clone() const = 0;
};

class NoThreeDConfig : public simparm::Object, public ThreeDConfig {
    void set_traits( input::Traits<engine::Image>& ) const;
    simparm::Node& getNode() { return *this; }
  public:
    NoThreeDConfig() : simparm::Object("No3D", "No 3D") {}
    NoThreeDConfig* clone() const { return new NoThreeDConfig(); }
};

class ZhuangThreeDConfig : public simparm::Object, public ThreeDConfig {
    simparm::Entry< Eigen::Matrix< quantity< PerMicro, float >, 2, 1, Eigen::DontAlign > > widening;

    void set_traits( input::Traits<engine::Image>& ) const;
    simparm::Node& getNode() { return *this; }
    void registerNamedEntries() { push_back( widening ); }
  public:
    ZhuangThreeDConfig();
    ZhuangThreeDConfig(const ZhuangThreeDConfig&);
    ZhuangThreeDConfig* clone() const { return new ZhuangThreeDConfig(*this); }
};

class Config : public simparm::Object {
    friend class Check;

    typedef  Eigen::Matrix< quantity< si::nanolength, double >, 2, 1, Eigen::DontAlign > PSFSize;
    simparm::Entry<PSFSize> psf_size;
    simparm::NodeChoiceEntry< ThreeDConfig > three_d;
    traits::CuboidConfig cuboid_config;

  public:
    static traits::ImageResolution
        get( const FloatPixelSizeEntry::value_type& f );
    typedef input::chain::DefaultTypes SupportedTypes;

    Config();
    void registerNamedEntries();
    void set_traits( input::Traits<Localization>& ) const;
    void set_traits( input::Traits<engine::Image>& ) const;
    void read_traits( const input::Traits<engine::Image>& );
};

template <typename ForwardedType>
class ResolutionSetter 
: public Source<ForwardedType>, public input::Filter
{
    std::auto_ptr< Source<ForwardedType> > s;
    Config config;

  public:
    ResolutionSetter(
        std::auto_ptr< Source<ForwardedType> > backend,
        const Config& config ) 
        : Source<ForwardedType>( backend->getNode(), backend->flags ),
          s(backend), config(config) {}

    BaseSource& upstream() { return *s; }

    typedef typename Source<ForwardedType>::iterator iterator;
    typedef typename Source<ForwardedType>::TraitsPtr TraitsPtr;

    void dispatch(BaseSource::Messages m) { s->dispatch(m); }
    iterator begin() { return s->begin(); }
    iterator end() { return s->end(); }
    TraitsPtr get_traits();
};


class ChainLink 
: public input::chain::Filter, public simparm::TreeListener 
{
    typedef input::chain::DefaultVisitor< Config > Visitor;
    friend class input::chain::DelegateToVisitor;
    friend class Check;

    simparm::Structure<Config> config;
    simparm::Structure<Config>& get_config() { return config; }
    ContextRef context;

    class TraitMaker;

  protected:
    void operator()(const simparm::Event&);

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    ChainLink* clone() const { return new ChainLink(*this); }
    simparm::Node& getNode() { return config; }

    AtEnd traits_changed( TraitsRef r, Link* l);
    AtEnd context_changed( ContextRef r, Link* l);
    BaseSource* makeSource();
};

}

}
}

namespace boost {
namespace units {

std::string name_string(const dStorm::input::Resolution::PerMicro&);
std::string symbol_string(const dStorm::input::Resolution::PerMicro&);

}
}

#endif
