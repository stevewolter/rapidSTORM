#ifndef NONLINFIT_PLANE_DATAFACADE_HPP
#define NONLINFIT_PLANE_DATAFACADE_HPP

#include "DataFacade.h"
#include <boost/iterator/iterator_facade.hpp>

#include <iostream>

namespace nonlinfit {
namespace plane {

template <typename CoreData>
DataFacade<CoreData>::DataFacade() 
: current_point(0) {}

template <typename CoreData>
void DataFacade<CoreData>::push_back( const value_type& point ) {
    if ( current_point == 0 )
        data.push_back( DataRow() );
    this->set( data.back(), current_point, point );
    current_point = (current_point + 1) % ChunkSize;
}

template <typename CoreData>
class DataFacade<CoreData>::const_iterator
: public boost::iterator_facade<
    const_iterator,
    value_type,
    boost::forward_traversal_tag >
{
    const CoreData& xs;
    typename Data::const_iterator chunk;
    int pos_in_chunk;
    mutable value_type point;
    
    friend class ::boost::iterator_core_access;
    void increment() {
        ++pos_in_chunk;
        if ( pos_in_chunk >= ChunkSize ) {
            pos_in_chunk = 0;
            ++chunk;
        }
    }

    value_type& dereference() const { 
        point = xs.get( *chunk, pos_in_chunk );
        return point;
    }

    bool equal( const const_iterator& o ) const {
        return chunk == o.chunk && pos_in_chunk == o.pos_in_chunk;
    }

public:
    const_iterator( const CoreData& d, typename Data::const_iterator chunk, int pos_in_chunk ) 
        : xs(d), chunk(chunk), pos_in_chunk(pos_in_chunk) {}
};

template <typename CoreData>
typename DataFacade<CoreData>::const_iterator
DataFacade<CoreData>::begin() const
    { return const_iterator( *this, data.begin(), 0 ); }

template <typename CoreData>
typename DataFacade<CoreData>::const_iterator
DataFacade<CoreData>::end() const { 
    return const_iterator( *this, data.end() - ( (current_point == 0) ? 0 : 1 ), current_point );
}

template <typename CoreData>
void DataFacade<CoreData>::pad_last_chunk() {
    if ( current_point == 0 )
        /* Everything is OK */ ;
    else if ( current_point < ChunkSize/2 ) {
        data.pop_back();
        current_point = 0;
    } else {
        const_iterator from( *this, data.end()-1, current_point - ChunkSize/2 );
        while ( current_point != 0 ) {
            push_back( *from++ );
        }
    }
    assert( current_point == 0 );
}


}
}

#endif
