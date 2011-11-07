#ifndef DSTORM_OUTPUTS_MEMORYCACHE_CACHE_H
#define DSTORM_OUTPUTS_MEMORYCACHE_CACHE_H

#include <vector>
#include <dStorm/Localization.h>
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace memory_cache {

struct Store {
    typedef std::vector<Localization>::iterator loc_iter;
    typedef std::vector<Localization>::const_iterator const_loc_iter;

    virtual ~Store() {}
    virtual Store* clone() const = 0;
    virtual Store* make_empty_clone() const = 0;
    virtual void store(const_loc_iter from, const_loc_iter to) = 0;
    virtual void recall(int offset, loc_iter from, loc_iter to) const = 0;

    static boost::ptr_vector<Store> instantiate_necessary_caches
        ( const input::Traits<Localization>& );
};

}
}

namespace boost {
template <>
inline dStorm::memory_cache::Store* 
new_clone<dStorm::memory_cache::Store>
    ( const dStorm::memory_cache::Store& o )
    { return o.clone(); }
}

#endif
