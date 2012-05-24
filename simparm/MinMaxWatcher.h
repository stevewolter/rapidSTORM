#ifndef SIMPARM_MINMAXWATCHER_HH
#define SIMPARM_MINMAXWATCHER_HH

#include "MinMaxWatcher_decl.h"
#include "BoostOptional.h"
#include <boost/utility.hpp>

namespace simparm {

template <typename Type> class Attribute;

template <typename Type>
inline typename boost::enable_if< boost::is_fundamental<Type>, bool>::type
exceeds( const Type& a, const Type& b ) {
    return a > b;
}

template <typename Type>
inline typename boost::enable_if< boost::is_fundamental<Type>, bool>::type
falls_below( const Type& a, const Type& b ) {
    return a < b;
}

template <typename Bounder, typename Boundee, bool LowerBound>
class BoundWatcher
: public Attribute< Boundee >::ChangeWatchFunction,
  private boost::noncopyable
{
  private:
    Attribute< Bounder > &bound;
    typename Attribute< Boundee >::ChangeWatchFunction* prev;

  public:
    BoundWatcher( Attribute<Bounder>& value, 
                   typename Attribute< Boundee >::ChangeWatchFunction* prev)
        : bound(value), prev(prev) { }

    /* The parameters are not used since we don't know whether we get 
     * the callback from value, min or max */
    bool operator()(const Boundee& from, const Boundee& to) {
        bool success = ( prev == NULL || (*prev)(from,to) ) &&
            ! ( (LowerBound) ? exceeds(bound(), to) : falls_below(bound(), to) );
        return success;
    }
};

}

#endif 
