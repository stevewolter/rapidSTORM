#include "debug.h"
#include "input/Link.h"

#ifdef VERBOSE
#include "engine/Image.h"
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"
#include "localization/Traits.h"
#include "Localization.h"
#include "input/MetaInfo.h"
#endif

#include "engine/InputTraits.h"
#include "input/MetaInfo.h"

namespace dStorm {
namespace input {

Link::Link() 
{
    DEBUG("Created " << this);
}

Link::Link(const Link& o) 
: meta_info(o.meta_info) {}

Link::~Link() {
    DEBUG("Destructed " << this);
}

void Link::update_current_meta_info( TraitsRef new_traits ) 
{
    assert( new_traits.get() );
    if ( new_traits->provides< dStorm::engine::ImageStack >() )
        assert( new_traits->traits< dStorm::engine::ImageStack >()->plane_count() > 0 );
    meta_info = new_traits;
    DEBUG("Publishing traits " << meta_info.get() << " for " << this);
    meta_info_signal( meta_info );
}

void Terminus::insert_new_node( std::auto_ptr<Link> l, Place ) {
    throw std::logic_error("No insertion point found");
}

Link::Connection
Link::notify( const TraitsSignal::slot_type& whom ) { 
    DEBUG(this << " adding notification");
    return Connection( new boost::signals2::scoped_connection(
            meta_info_signal.connect( whom ) ) ); 
}


std::auto_ptr<BaseSource> Link::make_source() {
    return std::auto_ptr<BaseSource>(makeSource());
}

}
}

