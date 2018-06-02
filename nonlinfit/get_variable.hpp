#ifndef NONLINFIT_GET_VARIABLE_HPP
#define NONLINFIT_GET_VARIABLE_HPP

#include <boost/mpl/at.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/size.hpp>

namespace nonlinfit {

struct get_variable {
    typedef void result_type;
    template <typename Expression, typename Position, typename IndexType>
    void operator()( const Expression& m, Position& p, IndexType ) {
        typedef typename boost::mpl::at<typename Expression::Variables, IndexType>::type Parameter;
        p[ IndexType::value ] = m( Parameter() );
    }

    template <typename Expression, typename Position>
    static void fill_vector(const Expression& m, Position& p) {
	static const int VariableCount = boost::mpl::size<typename Expression::Variables>::value;
        boost::mpl::for_each<boost::mpl::range_c<int, 0, VariableCount>>(
            boost::bind(get_variable(), boost::cref(m), boost::ref(p), boost::placeholders::_1));
    }
};

}

#endif
