#ifndef DSTORM_INPUT_JOIN_SPATIAL_HPP
#define DSTORM_INPUT_JOIN_SPATIAL_HPP

#include "iterator.hpp"
#include <dStorm/dimension_names.h>

namespace dStorm {
namespace input {
namespace join {

template <int Dim> struct spatial_tag {
    static std::string get_name() { return "Spatial" + spatial_dimension_name<Dim>(); }
    static std::string get_desc() { return "Spatially in " + spatial_dimension_name<Dim>() + " dimension"; }
};

template <int Dim>
struct merge_traits< engine::Image, spatial_tag<Dim> > : public traits_merger<engine::Image>
{
    typename traits_merger<engine::Image>::result_type
        operator()( const typename traits_merger<engine::Image>::argument_type& ) const;
};

template <int Dim>
struct merge_data< engine::Image, spatial_tag<Dim> >
{
    typedef engine::Image result_type;

    engine::Image operator()( 
        const input::Traits<engine::Image>& traits,
        const std::vector< input::Source<engine::Image>::iterator >& s,
        spatial_tag<Dim> ) const;
};

}
}
}

#endif
