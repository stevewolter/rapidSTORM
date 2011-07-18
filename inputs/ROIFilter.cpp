#include "debug.h"
#include "ROIFilter.h"
#include <simparm/OptionalEntry_impl.hh>
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

template <>
template <typename Type>
bool DefaultVisitor<ROIFilter::Config>::operator()( Traits<Type>& traits )
{
    typedef Localization::ImageNumber::Traits ImT;
    traits.image_number().range().first = config.first_frame();
    if ( config.last_frame().is_set() )
        traits.image_number().range().second = config.last_frame();

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
        || config.last_frame().is_set()
        || config.which_plane() != -1 )
    {
        boost::optional<int> plane;
        if ( config.which_plane() != -1 )
            plane = config.which_plane();
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
}

ChainLink::AtEnd ChainLink::traits_changed( TraitsRef c, Link* l ) { 
    const traits::Optics<3>* t = 
        dynamic_cast< const traits::Optics<3>* >(c.get());
    if ( t ) {
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


}
}
