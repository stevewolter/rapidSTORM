#ifndef NONLINFIT_FUNCTORS_APPEND_H
#define NONLINFIT_FUNCTORS_APPEND_H

#include <boost/mpl/insert_range.hpp>
#include <boost/mpl/end.hpp>

namespace nonlinfit {

/** Append one Boost.MPL sequence to another.
 *  This metafunction returns a Boost.MPL sequence that consists of
 *  the elements of A followed by the elements of B.
 *  This is the sequence operator variant of boost::mpl::joint_view
 *  and was implemented to save compiler memory.
 **/
template <typename A, typename B>
struct append {
    typedef typename boost::mpl::insert_range< 
        A, 
        typename boost::mpl::end<A>::type, 
        B 
    >::type type;
};

}

#endif
