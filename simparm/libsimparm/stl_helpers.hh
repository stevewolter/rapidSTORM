#ifndef STL_HELPERS
#define STL_HELPERS
#include <algorithm>
#include <functional>

namespace nstd {
    template <typename Container, typename Element>
    void erase( Container& c, Element e ) 
    {
        c.erase( std::remove(c.begin(), c.end(), e), c.end() );
    }

    template <typename Container, typename Operation>
    Operation for_each( Container& c, Operation o ) 
    {
        return std::for_each( c.begin(), c.end(), o );
    }

};
#endif
