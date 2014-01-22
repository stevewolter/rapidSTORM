#include "debug.h"
#include "Link.h"

#ifdef VERBOSE
#include "dStorm/engine/Image.h"
#include "dStorm/output/LocalizedImage.h"
#include "dStorm/output/LocalizedImage_traits.h"
#include "dStorm/localization/Traits.h"
#include "dStorm/Localization.h"
#include "MetaInfo.h"
#endif

#include <dStorm/engine/InputTraits.h>
#include "MetaInfo.h"

namespace dStorm {
namespace input {

Link::Link() 
{
    DEBUG("Created " << this);
}

Link::Link(const Link& o) 
: meta_info(o.meta_info)
{
    DEBUG("Cloned " << this << " named " << o.name() << " to have context " << meta_info.get());
}

Link::~Link() {
    DEBUG("Destructed " << this);
}

void Link::update_current_meta_info( TraitsRef new_traits ) 
{
    assert( new_traits.get() );
    if ( new_traits->provides< dStorm::engine::ImageStack >() )
        assert( new_traits->traits< dStorm::engine::ImageStack >()->plane_count() > 0 );
    meta_info = new_traits;
    DEBUG("Publishing traits for " << this);
    meta_info_signal( meta_info );
}

void Terminus::insert_new_node( std::auto_ptr<Link> l, Place ) {
        throw std::logic_error("No insertion point found for " + l->name());
    }


Link::Connection
Link::notify( const TraitsSignal::slot_type& whom ) { 
    DEBUG(this << " adding notification");
    return Connection( new boost::signals2::scoped_connection(
            meta_info_signal.connect( whom ) ) ); 
}


std::string Link::name() const { throw std::logic_error("Not implemented"); }

std::auto_ptr<BaseSource> Link::make_source() {
    return std::auto_ptr<BaseSource>(makeSource());
}

}
}

