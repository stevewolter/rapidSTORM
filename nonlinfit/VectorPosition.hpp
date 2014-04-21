#ifndef NONLINFIT_EXPRESSIONMOVER_HPP
#define NONLINFIT_EXPRESSIONMOVER_HPP

#include "nonlinfit/VectorPosition.h"
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/at.hpp>

namespace nonlinfit {

/** Functor for extracting a single variable from an Lambda.
 *  Gets an instance of boost::mpl::int_ and sets the corresponding 
 *  scalar in the Position to the unitless value of the indexed
 *  parameter in the Lambda. */
template <class Lambda_>
struct VectorPosition<Lambda_>::get_variable {
    typedef void result_type;
    template <typename IndexType>
    void operator()( const Lambda_& m, Evaluation<double>::Vector& p, IndexType ) {
        typedef typename boost::mpl::at<Variables, IndexType>::type Parameter;
        p[ IndexType::value ] = m( Parameter() );
    }
};

/** Functor for implanting a single variable into an Lambda.
 *  Gets an instance of boost::mpl::int_ and writes the corresponding 
 *  scalar in the Position to the unitless value of the indexed
 *  parameter in the Lambda. */
template <class Lambda_>
struct VectorPosition<Lambda_>::set_variable {
    typedef void result_type;
    template <typename IndexType>
    void operator()( Lambda_& m, const Evaluation<double>::Vector& p, IndexType ) {
        typedef typename boost::mpl::at<Variables, IndexType>::type Parameter;
        m( Parameter() ) = p[ IndexType::value ];
    }
};

template <class _Lambda>
void VectorPosition<_Lambda>::get_position( Position& p ) const
{
    boost::mpl::for_each< boost::mpl::range_c<int,0,VariableCount> >( 
        boost::bind(get_variable(), boost::cref(expression), boost::ref(p), _1) );
}

template <class _Lambda>
void VectorPosition<_Lambda>::set_position( const Position& p )
{
    boost::mpl::for_each< boost::mpl::range_c<int,0,VariableCount> >( 
        boost::bind(set_variable(), boost::ref(expression), boost::cref(p), _1) );
}

}

#endif
