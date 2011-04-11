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

LayerConfig::LayerConfig(int number)
: simparm::Object("InputLayer" + boost::lexical_cast<std::string>(number), 
                  "Input layer " + boost::lexical_cast<std::string>(number)),
  is_first_layer(number==0),
  pixel_size_x("PixelSizeInNMX", "Size of one input pixel [x]"),
  pixel_size_y("PixelSizeInNMY", "Size of one input pixel [y]"),
  z_position("ZPosition", "Z position", 0 * boost::units::si::nanometre),
  micro_alignment("MicroAlignmentFile", "Plane Alignment file")
{
    z_position.setHelp("Z position of this layer in sample space relative to the first layer");
    if ( is_first_layer ) {
        z_position.viewable = z_position.editable = false; 
	micro_alignment.viewable = micro_alignment.editable = false;
    }
}

void LayerConfig::set_traits( OpticalInfo<2>& t ) const
{
    DEBUG("Setting optical for a layer, pixel size in x is set: " << pixel_size_x().is_set() );
    if ( pixel_size_x().is_set() ) t.resolution[0] = Config::get(*pixel_size_x());
    if ( pixel_size_y().is_set() ) t.resolution[1] = Config::get(*pixel_size_y());
    assert( ! pixel_size_x().is_set() || (t.resolution[0].is_set() && t.resolution[0]->is_in_dpm()) );
    t.tmc = std::vector<float>();
    for ( Transmissions::const_iterator i = transmissions.begin(); i != transmissions.end(); ++i)
        t.tmc->push_back( i->value() );
    t.z_position = z_position() * si::metre / (1E9 * si::nanometre);
}

void LayerConfig::set_number_of_fluorophores(int number)
{
    while ( int(transmissions.size()) < number ) {
       std::string i = boost::lexical_cast<std::string>(transmissions.size());
       transmissions.push_back( new simparm::DoubleEntry("Transmission" + i,
         	"Transmission of fluorophore " + i) );
        push_back( transmissions.back() );
    }

    for (Transmissions::iterator i = transmissions.begin(); i != transmissions.end(); ++i)
	i->viewable = (i - transmissions.begin()) < number;
}

void LayerConfig::registerNamedEntries() {
    push_back( pixel_size_x );
    push_back( pixel_size_y );
    push_back( z_position );
    for (Transmissions::iterator i = transmissions.begin(); i != transmissions.end(); ++i)
        push_back( *i );
}

void Config::set_traits( input::Traits<engine::Image>& t ) const
{
    t.psf_size().x() = 400E-9 * boost::units::si::meter;
    t.psf_size().y() = 400E-9 * boost::units::si::meter;
    if ( int(layers.size()) < t.plane_count() )
       throw std::logic_error("Input announced too few planes");
    for ( int i = 0; i <  t.plane_count(); ++i ) {
       DEBUG("Setting optical traits for layer " << i);
       t.plane(i).resolution[0] = Config::get(pixel_size_x());
       assert( t.plane(i).resolution[0].is_set() && t.plane(i).resolution[0]->is_in_dpm() );
       t.plane(i).resolution[1] = Config::get(pixel_size_y());
       layers[i].set_traits( t.plane(i) ); 
    }
}

#if 0
void FluorophoreConfig::FluorophoreConfig(int number)
: simparm::Object("Fluorophore" + boost::lexical_cast<std::string>(number), 
                  "Fluorophore " + boost::lexical_cast<std::string>(number)),
#endif

Config::Config()
: simparm::Object("Optics", "Optical parameters"),
  pixel_size_x("PixelSizeInNMX", "Size of one input pixel [x]",
                   105.0f * boost::units::si::nanometre / camera::pixel),
  pixel_size_y("PixelSizeInNMY", "Size of one input pixel [y]",
                   105.0f * boost::units::si::nanometre / camera::pixel),
  psf_size_x("PSFX", "PSF FWHM in X",
                  400.0 * boost::units::si::nanometre),
  psf_size_y("PSFY", "PSF FWHM in Y",
                  400.0 * boost::units::si::nanometre)
{
    layers.push_back( new LayerConfig(0) );
}

void Config::registerNamedEntries() {
    push_back( pixel_size_x );
    push_back( pixel_size_y );
    push_back( psf_size_x );
    push_back( psf_size_y );
}

ChainLink::ChainLink() 
{
    receive_changes_from_subtree( config );
}

ChainLink::ChainLink(const ChainLink& o) 
: Filter(o),
  config(o.config), context(o.context)
{
    receive_changes_from_subtree( config );
}

ChainLink::AtEnd ChainLink::traits_changed( TraitsRef c, Link* l ) { 
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
    return std::auto_ptr<chain::Forwarder>( new ChainLink() );
}

ImageResolution
Config::get( const FloatPixelSizeEntry::value_type& f ) {
    boost::units::quantity< boost::units::divide_typeof_helper<
        boost::units::si::length,camera::length>::type, float > q1;
    q1 = (f / (1E9 * boost::units::si::nanometre) * boost::units::si::metre);
    return q1;
}

void Config::set_traits( input::Traits<Localization>& t ) const {
    t.position().resolution().x() = 1.0f / (pixel_size_x() / (1E9f * si::nanometre) * si::metre) ;
    t.position().resolution().y() = 1.0f / (pixel_size_y() / (1E9f * si::nanometre) * si::metre) ;
}

}
}
}
