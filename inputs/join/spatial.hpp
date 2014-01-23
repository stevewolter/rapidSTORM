#ifndef DSTORM_INPUT_JOIN_SPATIAL_HPP
#define DSTORM_INPUT_JOIN_SPATIAL_HPP

#include "iterator.hpp"
#include <dStorm/dimension_names.h>
#include <dStorm/engine/Image.h>

namespace dStorm {
namespace input {
namespace join {

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

    bool operator()( 
        const input::Traits<engine::ImageStack>& traits,
        const std::vector< std::unique_ptr<input::Source<engine::ImageStack>> >& s,
        spatial_tag<2>,
        engine::ImageStack* result) const;
};

}
}
}

#endif
