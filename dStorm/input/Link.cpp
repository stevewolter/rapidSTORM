#include "debug.h"
#include "Link.h"
#include <simparm/Node.hh>

#ifdef VERBOSE
#include "dStorm/engine/Image.h"
#include "dStorm/ImageTraits.h"
#include "MetaInfo.h"
#endif

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
    meta_info = new_traits;
    meta_info_signal( meta_info );
}

void Terminus::insert_new_node( std::auto_ptr<Link> l, Place ) {
        throw std::logic_error("No insertion point found for " + l->description());
    }

std::string Link::name() const { throw std::logic_error("Not implemented"); }

}
}

