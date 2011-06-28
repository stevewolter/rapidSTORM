#ifndef DSTORM_OUTPUTS_MEMORYCACHE_CACHE_H
#define DSTORM_OUTPUTS_MEMORYCACHE_CACHE_H

#include <dStorm/output/LocalizedImage.h>
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace output {
namespace memory_cache {

struct Interface {
    typedef LocalizedImage::iterator loc_iter;
    typedef LocalizedImage::const_iterator const_loc_iter;

    virtual ~Interface() {}
    virtual Interface* clone() const = 0;
    virtual Interface* make_empty_clone() const = 0;
    virtual void store(const_loc_iter from, const_loc_iter to) = 0;
    virtual void recall(int offset, loc_iter from, loc_iter to) const = 0;

    static boost::ptr_vector<Interface> instantiate_necessary_caches
        ( const input::Traits<LocalizedImage>& );
};

}
}
}

namespace boost {
template <>
inline dStorm::output::memory_cache::Interface* 
new_clone<dStorm::output::memory_cache::Interface>
    ( const dStorm::output::memory_cache::Interface& o )
    { return o.clone(); }
}

#endif
