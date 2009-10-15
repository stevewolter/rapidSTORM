#ifndef DSTORM_TRACE
#define DSTORM_TRACE

#include <dStorm/Localization.h>
#include <data-c++/Vector.h>

namespace dStorm {
    /** A Cluster is a representation of a continuous sequence of 
     *  localizations that are thought to belong to the same fluorophore.
     **/
    struct Trace : public data_cpp::Vector<Localization> {};
    typedef Trace Cluster;
}

#endif
