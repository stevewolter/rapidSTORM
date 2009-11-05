#ifndef DSTORM_TRACE
#define DSTORM_TRACE

#include <dStorm/output/Localization.h>
#include <dStorm/data-c++/Vector.h>

namespace dStorm {
    /** A Cluster is a representation of a continuous sequence of 
     *  localizations that are thought to belong to the same fluorophore.
     **/
    struct Trace : public data_cpp::Vector<Localization> {};
    typedef Trace Cluster;
}

#endif
