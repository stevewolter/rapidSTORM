#include "debug.h"
#include "ResolutionSetter.h"
#include <dStorm/input/Source_impl.h>
#include <dStorm/input/chain/Context_impl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/chain/Filter_impl.h>
#include <dStorm/input/chain/DefaultFilterTypes.h>
#include <dStorm/ImageTraits_impl.h>
#include <boost/lexical_cast.hpp>
#include <simparm/OptionalEntry_impl.hh>
#include <boost/lexical_cast.hpp>
#include <boost/variant/get.hpp>

namespace dStorm {
namespace input {

namespace chain {

template <>
template <typename Type>
bool DefaultVisitor<Resolution::Config>::operator()( Traits<Type>& traits )
{
    config.set_traits( traits );
    return true;
}

template <>
template <typename Type>
bool DefaultVisitor<Resolution::Config>::operator()( std::auto_ptr< Source<Type> > s )
{
    assert( s.get() );
    new_source.reset( new Resolution::ResolutionSetter<Type>(s, config) );
    return true;
}

}

namespace Resolution {

using namespace chain;

template <typename ForwardedType>
typename ResolutionSetter<ForwardedType>::TraitsPtr
ResolutionSetter <ForwardedType>::get_traits()
{
    DEBUG("Setting traits in ResolutionSetter");
    TraitsPtr rv = s->get_traits();
    config.set_traits(*rv);
    DEBUG(this << " set traits in ResolutionSetter");
    return rv;
}

void Config::set_traits( input::Traits<engine::Image>& t ) const
{
    DEBUG("Setting traits in ResolutionSetter, input is " << t.size.transpose());
    static_cast<traits::Optics<3>&>(t) = cuboid_config.make_traits();
    t.psf_size().x() = quantity<si::length>(psf_size_x() / si::nanometre * 1E-9 * si::metre) / 2.35;
    t.psf_size().y() = quantity<si::length>(psf_size_y() / si::nanometre * 1E-9 * si::metre) / 2.35;
    if ( widening_x().is_set() && widening_y().is_set() ) {
        traits::Zhuang3D zhuang;
        zhuang.widening[0] = quantity<traits::Zhuang3D::Unit>( *widening_x() );
        zhuang.widening[1] = quantity<traits::Zhuang3D::Unit>( *widening_y() );
        t.depth_info = zhuang;
        const_cast< traits::CuboidConfig& >(cuboid_config).set_3d_availability(true);
    } else {
        t.depth_info = traits::No3D();
        const_cast< traits::CuboidConfig& >(cuboid_config).set_3d_availability(false);
    }
}

#if 0
void FluorophoreConfig::FluorophoreConfig(int number)
: simparm::Object("Fluorophore" + boost::lexical_cast<std::string>(number), 
                  "Fluorophore " + boost::lexical_cast<std::string>(number)),
#endif

Config::Config()
: simparm::Object("Optics", "Optical parameters"),
  psf_size_x("PSFX", "PSF FWHM in X",
                  493.5 * boost::units::si::nanometre),
  psf_size_y("PSFY", "PSF FWHM in Y",
                  493.5 * boost::units::si::nanometre),
  widening_x("XDefocusConstant", "Speed of PSF std. dev. growth in X"),
  widening_y("YDefocusConstant", "Speed of PSF std. dev. growth in Y")
{
}

void Config::registerNamedEntries() {
    push_back( psf_size_x );
    push_back( psf_size_y );
    push_back( widening_x );
    push_back( widening_y );
    cuboid_config.registerNamedEntries();
    push_back( cuboid_config );
}

ChainLink::ChainLink() 
{
    DEBUG("Making ResolutionSetter chain link");
    receive_changes_from_subtree( config );
}

ChainLink::ChainLink(const ChainLink& o) 
: Filter(o),
  config(o.config), context(o.context)
{
    receive_changes_from_subtree( config );
}

ChainLink::AtEnd ChainLink::traits_changed( TraitsRef c, Link* l ) { 
    if ( c.get() && c->provides< dStorm::engine::Image >() ) {
        config.read_traits( *c->traits< dStorm::engine::Image >() );
    }
    return input::chain::DelegateToVisitor::traits_changed(*this, c, l);
}

chain::Link::AtEnd ChainLink::context_changed( ContextRef c, Link *l )
{
    this->context = c;
    return input::chain::DelegateToVisitor::context_changed(*this, c, l);
}

void ChainLink::operator()(const simparm::Event& e)
{
    if ( e.cause == simparm::Event::ValueChanged) {
	ost::MutexLock lock( global_mutex() );
        if ( ! context.get() ) return;
        DefaultVisitor<Config> m(config);
        visit_context( m, context );
        notify_of_context_change( context );
        MetaInfo::ConstPtr t( current_traits() );
        visit_traits( m, t );
        notify_of_trait_change( t );
    } else 
	TreeListener::add_new_children(e);
}

BaseSource* ChainLink::makeSource() { 
    DefaultVisitor<Config> visitor(config);
    return specialize_source( visitor, Forwarder::makeSource() );
}

std::auto_ptr<chain::Forwarder> makeLink() {
    DEBUG("Making resolution chain link");
    return std::auto_ptr<chain::Forwarder>( new ChainLink() );
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

void Config::read_traits( const input::Traits<engine::Image>& t ) {
    cuboid_config.set_entries_to_traits( t, t.fluorophores.size() );
}

}
}
}
