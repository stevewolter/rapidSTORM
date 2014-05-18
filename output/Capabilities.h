#ifndef DSTORM_OUTPUT_CAPABILITIES_H
#define DSTORM_OUTPUT_CAPABILITIES_H

#include <bitset>

namespace dStorm {
namespace output {

class Capabilities 
{
    std::bitset<3> base;
    int src_lvls;
    Capabilities(const std::bitset<3>& base, int levels_of_cl_s = 0) 
        : base(base), src_lvls(levels_of_cl_s)
            {}

  public:
    Capabilities() : src_lvls(0) {}

    enum Capability {
        SourceImage,
        InputBuffer,
        ClustersWithSources
    };

    void set_intransparency_for_source_data() { base.reset(); }

    Capabilities& set_source_image(bool has_cap = true)
        { base.set( SourceImage, has_cap ); return *this; }
    Capabilities& set_input_buffer(bool has_cap = true)
        { base.set( InputBuffer, has_cap ); return *this; }
    Capabilities& set_cluster_sources(bool has_cap = true)
        { src_lvls += (has_cap) ? 1 : 0; return *this; }
    Capabilities& remove_cluster_sources()
        { src_lvls = std::min(src_lvls-1, 0); return *this; }

    bool test( int field ) const { 
        return (field != ClustersWithSources) 
            ? base.test(field) : (src_lvls > 0); }
    bool test_cluster_sources() const { return test(ClustersWithSources);}

    Capabilities operator~() const 
        { return Capabilities( ~base, !src_lvls); }
    bool any() const { return base.any() || test(ClustersWithSources); }
    Capabilities operator|( const Capabilities& o) const
        { return Capabilities( base | o.base, 
                               std::max(src_lvls, o.src_lvls) ); }
    Capabilities operator&( const Capabilities& o) const
        { return Capabilities( base & o.base, 
                               std::min(src_lvls, o.src_lvls) ); }

    Capabilities& operator|=( const Capabilities& o ) {
        base |= o.base;
        src_lvls = std::max(src_lvls, o.src_lvls);
        return *this;
    }
    Capabilities& operator&=( const Capabilities& o ) {
        base &= o.base;
        src_lvls = std::min(src_lvls, o.src_lvls);
        return *this;
    }
};

std::ostream& operator<<(std::ostream&, Capabilities caps);

}
}

#endif
