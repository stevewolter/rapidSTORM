#ifndef DSTORM_INPUT_JOIN_TEMPORAL_HPP
#define DSTORM_INPUT_JOIN_TEMPORAL_HPP

#include <deque>

#include <dStorm/engine/Image.h>
#include <dStorm/input/Traits.h>
#include <boost/optional/optional.hpp>

#include "iterator_decl.hpp"

namespace dStorm {
namespace input {
namespace join {

struct temporal_tag {
    static std::string get_name() { return "Temporal"; }
    static std::string get_desc() { return "In time"; }
};

template <typename Type>
struct merge_traits< Type, temporal_tag > : public traits_merger<Type>
{
    typename traits_merger<Type>::result_type 
        operator()( const typename traits_merger<Type>::argument_type& ) const;
};

template <typename Type>
struct iterator<Type,temporal_tag>
: public boost::iterator_facade< iterator<Type,temporal_tag>, Type, boost::forward_traversal_tag >
{
    typedef input::Source<Type> Base;
    typedef std::vector< boost::shared_ptr<const input::Traits<Type> > > Traits;
    typedef std::vector< boost::shared_ptr<Base> > Sources;

    struct InputPart {
        boost::optional<frame_index> length;
        typename Base::iterator begin, end;
        boost::shared_ptr<Base> source;

        InputPart( const boost::shared_ptr<Base>& base, const input::Traits<Type>& );
        bool operator==( const InputPart& o ) const;
    };

    typedef std::deque< InputPart > Parts;
    Parts bases;
    mutable boost::optional<Type> joined;
    mutable frame_index max_frame_count;
    frame_index current_offset;

    friend class boost::iterator_core_access;
    void increment();
    bool equal( const iterator& other ) const;
    Type& dereference() const;

    void pop_empty_parts();

  public:
    iterator( const typename Base::TraitsPtr&, const Traits&, Sources&, bool end );
};

}
}
}

#endif
