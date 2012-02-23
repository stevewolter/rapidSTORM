#include "debug.h"
#include "Link.h"
#include <simparm/Node.hh>

#ifdef VERBOSE
#include "dStorm/engine/Image.h"
#include "dStorm/output/LocalizedImage.h"
#include "dStorm/output/LocalizedImage_traits.h"
#include "dStorm/localization/Traits.h"
#include "dStorm/Localization.h"
#include "dStorm/ImageTraits.h"
#include "MetaInfo.h"
#endif

#include <dStorm/engine/InputTraits.h>
#include "MetaInfo.h"

namespace dStorm {
namespace input {

Link::Link() 
{
}

Link::Link(const Link& o) 
: meta_info(o.meta_info)
{
    DEBUG("Cloned " << o.name() << " to have context " << meta_info.get());
}

Link::~Link() {
}

void Link::update_current_meta_info( TraitsRef new_traits ) 
{
    if ( new_traits.get() && new_traits->provides< dStorm::engine::ImageStack >() )
        assert( new_traits->traits< dStorm::engine::ImageStack >()->plane_count() > 0 );
    meta_info = new_traits;
    meta_info_signal( meta_info );
}

void Terminus::insert_new_node( std::auto_ptr<Link> l, Place ) {
        throw std::logic_error("No insertion point found for " + l->description());
    }

std::string Link::name() const { throw std::logic_error("Not implemented"); }

std::auto_ptr<BaseSource> Link::make_source() {
    return std::auto_ptr<BaseSource>(makeSource());
}

}
}

