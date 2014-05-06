#ifndef NONLINFIT_SET_VARIABLE_HPP
#define NONLINFIT_SET_VARIABLE_HPP

#include <boost/mpl/at.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/size.hpp>

namespace nonlinfit {

struct set_variable {
    typedef void result_type;
    template <typename Expression, typename Position, typename IndexType>
    void operator()( const Position& p, Expression& m, IndexType ) {
        typedef typename boost::mpl::at<typename Expression::Variables, IndexType>::type Parameter;
        m( Parameter() ) = p[ IndexType::value ];
    }

    template <typename Expression, typename Position>
    static void read_vector(const Position& p, Expression& m) {
	static const int VariableCount = boost::mpl::size<typename Expression::Variables>::value;
        boost::mpl::for_each<boost::mpl::range_c<int,0,VariableCount>>(
            boost::bind(set_variable(), boost::cref(p), boost::ref(m), _1));
    }
};

}

#endif
