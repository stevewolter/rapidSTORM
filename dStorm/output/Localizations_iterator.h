#ifndef DSTORM_LOCALIZATIONS_ITERATOR_H
#define DSTORM_LOCALIZATIONS_ITERATOR_H

#include "Localizations.h"
#include "LocalizedImage.h"
#include <boost/iterator/iterator_facade.hpp>
#include <iterator>

namespace dStorm {
namespace output {

class Localizations::_iterator  
: public boost::iterator_facade< 
    _iterator, const Localization, std::bidirectional_iterator_tag >
{
    friend class Localizations;
    friend class boost::iterator_core_access;

    typedef ImageSequence::const_iterator L1;
    typedef ImageSequence::const_iterator::value_type::const_iterator L2;

    L1 l1, e; L2 l2; 
    _iterator( L1 l1, L1 e ) : l1(l1), e(e) 
        { if ( l1 != e ) l2 = l1->begin(); else l2 = L2(); } 
 
    void increment() {  
        ++l2;  
        while ( l2 == l1->end() && l1 != e ) { 
            ++l1; 
            if ( l1 != e ) l2 = l1->begin(); else l2 = L2(); 
        } 
    } 
    const Localization& dereference() const { return *l2; } 
    void decrement() {  
        while ( l2 == l1->begin() ) { 
            --l1; 
            l2 = l1->end(); 
            --l2; 
        } 
    } 
    bool equal( const _iterator& o ) const {
        return o.l1 == l1 && l2 == o.l2; 
    } 
  public: 
    _iterator( const iterator& o ) : l1(o.l1), l2(o.l2) {} 
};

Localizations::iterator Localizations::begin()
    { return Localizations::iterator(localizations.begin(), localizations.end()); }
Localizations::iterator Localizations::end()
    { return Localizations::iterator(localizations.end(), localizations.end()); }
Localizations::const_iterator Localizations::begin() const
    { return Localizations::const_iterator(localizations.begin(), localizations.end()); }
Localizations::const_iterator Localizations::end() const
    { return Localizations::const_iterator(localizations.end(), localizations.end()); }

}
}

#endif
