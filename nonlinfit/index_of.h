#ifndef NONLINFIT_INDEX_OF_H
#define NONLINFIT_INDEX_OF_H

#include <boost/mpl/begin.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>

namespace nonlinfit {

/** This metafunction returns the index of Parameter within List. */
template <typename List, typename Parameter>
struct index_of {
   BOOST_STATIC_ASSERT(( 
        boost::mpl::contains< List, Parameter >::type::value ));
   static const int value = boost::mpl::distance<
                typename boost::mpl::begin< List >::type,
                typename boost::mpl::find< List, Parameter >::type 
            >::type::value;
};

}

#endif
