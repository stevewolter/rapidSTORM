#ifndef DSTORM_INPUT_JOIN_SPATIAL_HPP
#define DSTORM_INPUT_JOIN_SPATIAL_HPP

#include "iterator.hpp"
#include <dStorm/dimension_names.h>
#include <dStorm/engine/Image.h>

namespace dStorm {
namespace input {
namespace join {

template <int Dim> struct spatial_tag {
    static std::string get_name() { return "Spatial" + spatial_dimension_name<Dim>(); }
    static std::string get_desc() { return "Spatially in " + spatial_dimension_name<Dim>() + " dimension"; }
};

template <>
struct merge_traits< engine::ImageStack, spatial_tag<2> >
: public traits_merger<engine::ImageStack>
{
    result_type operator()( const argument_type& ) const;
};

template <>
struct merge_data< engine::ImageStack, spatial_tag<2> >
{
    typedef engine::ImageStack result_type;

    engine::ImageStack operator()( 
        const input::Traits<engine::ImageStack>& traits,
        const std::vector< input::Source<engine::ImageStack>::iterator >& s,
        spatial_tag<2> ) const;
};

}
}
}

#endif
