#ifndef NONLINFIT_EXPRESSIONMOVER_HPP
#define NONLINFIT_EXPRESSIONMOVER_HPP

#include "VectorPosition.h"
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/at.hpp>

namespace nonlinfit {

/** Functor for extracting a single variable from an Lambda.
 *  Gets an instance of boost::mpl::int_ and sets the corresponding 
 *  scalar in the Position to the unitless value of the indexed
 *  parameter in the Lambda. */
template <class _Lambda>
struct VectorPosition<_Lambda>::get_variable {
    const Expression& m;
    Position& p;
    get_variable( Expression& m, Position& p ) : m(m), p(p) {}

    template <typename IndexType>
    void operator()( IndexType ) {
        typedef typename boost::mpl::at<Variables, IndexType>::type Parameter;
        p[ IndexType::value ] = m( Parameter() );
    }
};

/** Functor for implanting a single variable into an Lambda.
 *  Gets an instance of boost::mpl::int_ and writes the corresponding 
 *  scalar in the Position to the unitless value of the indexed
 *  parameter in the Lambda. */
template <class _Lambda>
struct VectorPosition<_Lambda>::set_variable {
    Expression& m;
    const Position& p;
    set_variable( Expression& m, const Position& p ) : m(m), p(p) {}

    template <typename IndexType>
    void operator()( IndexType ) {
        typedef typename boost::mpl::at<Variables, IndexType>::type Parameter;
        m( Parameter() ) = p[ IndexType::value ];
    }
};

template <class _Lambda>
void VectorPosition<_Lambda>::get_position( Position& p ) const
{
    boost::mpl::for_each< boost::mpl::range_c<int,0,VariableCount> >( 
        get_variable(expression, p) );
}

template <class _Lambda>
void VectorPosition<_Lambda>::set_position( const Position& p )
{
    boost::mpl::for_each< boost::mpl::range_c<int,0,VariableCount> >( 
        set_variable(expression, p) );
}

}

#endif
