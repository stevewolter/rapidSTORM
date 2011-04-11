#ifndef DSTORM_INPUT_RESOLUTIONSETTER_H
#define DSTORM_INPUT_RESOLUTIONSETTER_H

#include "debug.h"
#include <dStorm/input/Source.h>

#include <simparm/TreeCallback.hh>
#include <simparm/FileEntry.hh>
#include <simparm/OptionalEntry.hh>
#include <dStorm/UnitEntries/PixelSize.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/localization/Traits.h>
#include <simparm/Structure.hh>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Filter.h>
#include <dStorm/Image_decl.h>

namespace dStorm {
namespace input {

using namespace chain;

namespace Resolution {

class LayerConfig : public simparm::Object {
    const bool is_first_layer;
    simparm::OptionalEntry<boost::units::quantity<nanometer_pixel_size, float> >
	pixel_size_x, pixel_size_y;
    simparm::UnitEntry<boost::units::si::nanolength, double> z_position;
    simparm::FileEntry micro_alignment;
    typedef boost::ptr_vector< simparm::DoubleEntry > Transmissions;
    Transmissions transmissions;

  public:
    LayerConfig(int number);
    void set_traits( OpticalInfo<2>& t ) const;
    void registerNamedEntries();
    void set_number_of_fluorophores(int number);
};

class Config : public simparm::Object {
    friend class Check;

    FloatPixelSizeEntry pixel_size_x, pixel_size_y;
    simparm::UnitEntry< si::nanolength, double > psf_size_x, psf_size_y;
    boost::ptr_vector< LayerConfig > layers;

  public:
    static boost::units::quantity<camera::resolution, float>
        get( const FloatPixelSizeEntry::value_type& f );
    typedef input::chain::DefaultTypes SupportedTypes;

    Config();
    void registerNamedEntries();
    void set_traits( input::Traits<Localization>& ) const;
    void set_traits( input::Traits<engine::Image>& ) const;
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
    TraitsPtr get_traits() {
        TraitsPtr rv = s->get_traits();
        config.set_traits(*rv);
        return rv;
    }
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


#endif
