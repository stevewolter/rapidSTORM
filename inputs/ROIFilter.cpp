#include "debug.h"
#include "ROIFilter.h"
#include <simparm/Entry_Impl.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/input/chain/Filter_impl.h>
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <dStorm/input/chain/DefaultFilterTypes.h>
#include <boost/lexical_cast.hpp>

namespace dStorm {

namespace input {
namespace chain {

void delete_plane( Traits<engine::Image>& traits, int plane ) 
{
    std::swap( traits.planes[0], traits.planes[ plane ] );
    traits.planes.resize(1);
}
template <typename T>
void delete_plane( Traits<T>&, int ) {}

template <>
template <typename Type>
bool DefaultVisitor<ROIFilter::Config>::operator()( Traits<Type>& traits )
{
    typedef Localization::ImageNumber::Traits ImT;
    traits.image_number().range().first = config.first_frame();
    if ( config.last_frame().is_initialized() )
        traits.image_number().range().second = config.last_frame();
    if ( config.which_plane() != -1 ) {
        delete_plane( traits, config.which_plane() );
    }

    return true;
}

template <>
template <typename Type>
bool DefaultVisitor<ROIFilter::Config>::operator()( std::auto_ptr< Source<Type> > s )
{
    DEBUG("Making source for ROI filter");
    assert( s.get() );
    typedef ROIFilter::Source<Type> Filter;

    if ( config.first_frame() > 0 * camera::frame 
        || config.last_frame().is_initialized()
        || config.which_plane() != -1 )
    {
        boost::optional<int> plane;
        if ( config.which_plane() != -1 ) {
            plane = config.which_plane();
        }
        new_source.reset( new Filter( s, config.first_frame(), config.last_frame(), plane ) );
    } else
        new_source = s;
    DEBUG("Made source for ROI filter");
    return true;
}

}
}

namespace ROIFilter {

Config::Config() 
: simparm::Object("ROIFilter", "Image selection filter"),
  first_frame("FirstImage", "First image to load"),
  last_frame( "LastImage", "Last image to load" ),
  which_plane( "OnlyPlane", "Process only given plane" )
{
    which_plane.addChoice(-1, "AllPlanes", "All planes");
    first_frame.userLevel = simparm::Object::Intermediate;
    last_frame.userLevel = simparm::Object::Intermediate;
    which_plane.userLevel = simparm::Object::Expert;
}

ChainLink::AtEnd ChainLink::traits_changed( TraitsRef c, Link* l ) { 
    if ( c->provides< dStorm::engine::Image >() ) {
        boost::shared_ptr<const input::Traits<engine::Image> > t = 
            c->traits< engine::Image >();
        for (int i = config.which_plane.numChoices()-1; i < t->plane_count(); ++i) {
            std::string id = boost::lexical_cast<std::string>(i);
            config.which_plane.addChoice( i, "Plane" + id, "Plane " + id );
        }
        for (int i = t->plane_count(); i < config.which_plane.numChoices()-1; ++i)
            config.which_plane.removeChoice( i );
    }
    return input::chain::DelegateToVisitor::traits_changed( *this, c, l );
}

input::BaseSource* ChainLink::makeSource()
{
    DEBUG("Making source for ROI filter");
    input::BaseSource* rv = input::chain::DelegateToVisitor::makeSource(*this);
    DEBUG("Made source for ROI filter");
    return rv;
}

std::auto_ptr<input::chain::Filter> makeFilter() 
{
    return std::auto_ptr<input::chain::Filter> (new ChainLink());
}

void ChainLink::operator()( const simparm::Event& ) {
    if ( last_traits.get() ) {
        std::cerr << "Re-publishing traits" << std::endl;
        input::chain::DelegateToVisitor::traits_changed(*this, last_traits, NULL);
    }
}

ChainLink::ChainLink()
: simparm::Listener( simparm::Event::ValueChanged )
{
    receive_changes_from( config.which_plane.value );
}

ChainLink::ChainLink(const ChainLink& o)
: Filter(o), simparm::Listener( simparm::Event::ValueChanged ),
  last_traits(o.last_traits), config(o.config)
{
    receive_changes_from( config.which_plane.value );
}


}
}
