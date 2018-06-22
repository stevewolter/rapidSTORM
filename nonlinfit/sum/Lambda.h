#ifndef NONLINFIT_SUM_MODEL_H
#define NONLINFIT_SUM_MODEL_H

#include "nonlinfit/sum/fwd.h"

#include <Eigen/Core>
#include <boost/mpl/bind.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/iter_fold.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/include/at.hpp>
#include <nonlinfit/Lambda.h>
#include <nonlinfit/TermParameter.h>
#include <nonlinfit/access_parameters.hpp>
#include <nonlinfit/append.h>

#include <boost/static_assert.hpp>

namespace nonlinfit {
namespace sum {

/** A nonlinfit::Lambda representing the sum of lambdas.
 *  An instance of this class is a model of nonlinfit::Lamba that defines
 *  the same value as the sum of its parts. All parameters are considered to
 *  be distinct from each other.
 *
 *  \tparam Parts A Boost.MPL sequence of models of nonlinfit::Lambda.
 **/
template <typename Parts>
class Lambda 
{
    template <class Iterator>
    struct term_parameters {
        typedef typename boost::mpl::begin<Parts>::type Begin;
        typedef typename boost::mpl::distance< Begin, Iterator >::type Offset;
        template <typename Base>
        struct make_term_parameter {
            typedef TermParameter<Offset,Base> type;
        };
        typedef 
            typename boost::mpl::transform< 
                typename boost::mpl::deref<Iterator>::type::Variables,
                make_term_parameter<boost::mpl::_1>
            >::type
            type;
    };

  public:

    /** \sa nonlinfit::Lambda::Variables */
    typedef typename boost::mpl::iter_fold<
            Parts,
            boost::mpl::vector<>,
            nonlinfit::append<
                boost::mpl::_1,
                term_parameters< boost::mpl::_2 >
            >
        >::type
        Variables;

    /** Access the instance of the Index-ed Lambda in Parts. */
    template <typename Index>
    typename boost::mpl::at<Parts,Index>::type& get_part( Index ) 
        { return boost::fusion::at<Index>(parts); }
    /** Access the instance of the Index-ed Lambda in Parts. */
    template <typename Index>
    const typename boost::mpl::at<Parts,Index>::type& get_part( Index ) const 
        { return boost::fusion::at<Index>(parts); }

    /** \sa nonlinfit::Lambda::operator() */
    template <typename Term, typename Parameter>
    double operator()( TermParameter< Term, Parameter > ) const
        { return get_part( Term() )( Parameter() ); }
    /** \sa nonlinfit::Lambda::operator() */
    template <typename Term, typename Parameter>
    double& operator()( TermParameter< Term, Parameter > ) 
        { return get_part( Term() )( Parameter() ); }

    template <typename Term, typename Parameter>
    bool change_is_negligible( TermParameter< Term, Parameter > term, double from, double to ) const {
        return get_part( Term() ).change_is_negligible( Parameter(), from, to );
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    /** A Boost.Fusion vector with an instance of each class in Parts. */
    typename boost::fusion::result_of::as_vector< Parts >::type parts;
};

}

}

#endif
