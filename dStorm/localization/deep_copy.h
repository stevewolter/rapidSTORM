#ifndef DSTORM_LOCALIZATION_DEEP_COPY_H
#define DSTORM_LOCALIZATION_DEEP_COPY_H

namespace dStorm {

template <typename Container>
Localization* deep_copy( const Localization& l, Container& o )
{
    Localization n = l;
    if ( l.child ) n.child = deep_copy( *l.child );
    if ( l.sibling ) n.sibling = deep_copy( *l.sibling );
    o.push_back( n );
    return &o.back();
}

}

#endif
