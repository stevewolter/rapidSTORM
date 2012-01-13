#ifndef NONLINFIT_TERMINATORS_ALL_H
#define NONLINFIT_TERMINATORS_ALL_H

#include <functional>

namespace nonlinfit {
namespace terminators {

/** Combines two Terminator models.
 *  Both models receive all signals (methods with void signature),
 *  and should_continue_fitting() is decided by applying the Combination
 *  functor to the results of the individual should_continue_fitting()
 *  results. */
template < class T1, class T2, class Combination >
class Combined {
    T1 t1;
    T2 t2;
    Combination c;
  public:
    Combined( const T1& t1, const T2& t2, const Combination& c = Combination() )
        : t1(t1), t2(t2), c(c) {}

    void matrix_is_unsolvable() {
        t1.matrix_is_unsolvable();
        t2.matrix_is_unsolvable();
    }
    template <typename Position>
    void improved( const Position& current, const Position& shift ) {
        t1.improved( current, shift );
        t2.improved( current, shift );
    }
    void failed_to_improve( bool wrong ) {
        t1.failed_to_improve( wrong );
        t2.failed_to_improve( wrong );
    }
    bool should_continue_fitting() const { 
        return c( t1.should_continue_fitting(), t2.should_continue_fitting() ); }
};

/** Return a combined terminator that aborts fitting when a single of its
 *  constituents aborts fitting. */
template <class T1, class T2>
Combined<T1,T2,std::logical_and<bool> > all( const T1& t1, const T2& t2 ) 
    { return Combined<T1,T2,std::logical_and<bool> >(t1,t2); }
/** Return a combined terminator that aborts fitting when all of its
 *  constituents aborts fitting. */
template <class T1, class T2>
Combined<T1,T2,std::logical_or<bool> > any( const T1& t1, const T2& t2 ) 
    { return Combined<T1,T2,std::logical_or<bool> >(t1,t2); }


}
}

#endif
