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
#include <boost/lexical_cast.hpp>
#include <boost/variant/get.hpp>
#include <simparm/ChoiceEntry_Impl.hh>

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
    cuboid_config.set_traits(t);
    for (int i = 0; i < 2; ++i)
        t.psf_size()[i] = quantity<si::length>(psf_size()[i] / si::nanometre * 1E-9 * si::metre) / 2.35;

    three_d().set_traits( t );
    const_cast< traits::CuboidConfig& >(cuboid_config).set_3d_availability( 
        boost::get<traits::No3D>(&t.depth_info) == NULL );
}

void NoThreeDConfig::set_traits( input::Traits<engine::Image>& t ) const {
    t.depth_info = traits::No3D();
}

void ZhuangThreeDConfig::set_traits( input::Traits<engine::Image>& t ) const {
    traits::Zhuang3D zhuang;
    for (int i = 0; i < 2; ++i)
        zhuang.widening[i] = quantity<traits::Zhuang3D::Unit>( widening()[i] );
    t.depth_info = zhuang;
}

#if 0
void FluorophoreConfig::FluorophoreConfig(int number)
: simparm::Object("Fluorophore" + boost::lexical_cast<std::string>(number), 
                  "Fluorophore " + boost::lexical_cast<std::string>(number)),
#endif

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

Config::Config()
: simparm::Object("Optics", "Optical parameters"),
  psf_size("PSF", "PSF FWHM", PSFSize::Constant(500.0 * boost::units::si::nanometre)),
  three_d("ThreeD", "3D PSF model")
{
    psf_size.helpID = "PSF.FWHM";
    three_d.helpID = "3DType";
    three_d.addChoice( new NoThreeDConfig() );
    three_d.addChoice( new ZhuangThreeDConfig() );
    cuboid_config.set_3d_availability(false);
}

void Config::registerNamedEntries() {
    push_back( psf_size );
    push_back( three_d );
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
        DefaultVisitor<Config> m(config);
        if ( context.get() ) {
            visit_context( m, context );
            notify_of_context_change( context );
        }
        if ( current_traits().get() ) {
            MetaInfo::ConstPtr t( current_traits() );
            visit_traits( m, t );
            notify_of_trait_change( t );
        }
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

namespace boost {
namespace units {

std::string name_string(const dStorm::input::Resolution::PerMicro&)
{
    return "micrometer^-1";
}

std::string symbol_string(const dStorm::input::Resolution::PerMicro&)
{
    return "µm^-1";
}

}
}
