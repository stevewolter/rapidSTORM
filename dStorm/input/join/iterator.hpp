#ifndef DSTORM_INPUT_JOIN_ITERATOR_HPP
#define DSTORM_INPUT_JOIN_ITERATOR_HPP

#include "iterator_decl.hpp"
#include <dStorm/input/Source.h>

namespace dStorm {
namespace input {
namespace join {

template <typename Type, typename Tag>
struct merge_data;

template <typename Type, typename Tag>
struct iterator
: public boost::iterator_facade< iterator<Type,Tag>, Type, boost::forward_traversal_tag >
{
    typedef input::Source<Type> Base;
    typedef std::vector< boost::shared_ptr<Base> > Sources;
    typedef std::vector< typename  Base::iterator > Bases;
    Bases bases, ends;
    typename Base::TraitsPtr traits;
    mutable boost::optional<Type> joined;

    merge_data<Type,Tag> merger;

    friend class boost::iterator_core_access;
    void increment() {
        bool at_end = false;
        for (size_t i = 0; i < bases.size(); ++i) {
            ++bases[i];
            if ( bases[i] == ends[i] ) at_end = true;
        }
        if ( at_end ) bases = ends;
        joined.reset();
    }

    bool equal( const iterator& other ) const {
        return bases[0] == other.bases[0];
    }

    Type& dereference() const {
        if ( ! joined.is_initialized() )  
            joined = merger( *traits, bases, Tag() );
        return *joined;
    }

  public:
    iterator( const typename Base::TraitsPtr& traits, 
              const std::vector< boost::shared_ptr< const input::Traits<Type> > >&,
              Sources& sources, bool end )
    : traits(traits) {
        for (size_t i = 0; i < sources.size(); ++i) {
            bases.push_back( (end) ? sources[i]->end() : sources[i]->begin() );
            ends.push_back( sources[i]->end() );
        }
    }
};

}
}
}

#endif
