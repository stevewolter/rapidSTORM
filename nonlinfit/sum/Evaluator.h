#ifndef NONLINFIT_SUM_DERIVER_H
#define NONLINFIT_SUM_DERIVER_H

#include "nonlinfit/sum/fwd.h"
#include "nonlinfit/sum/Lambda.h"
#include <nonlinfit/Evaluator.h>
#include <nonlinfit/DerivationSummand.h>
#include <boost/bind/bind.hpp>
#include <boost/fusion/algorithm/iteration/fold.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/view/iterator_range.hpp>
#include <boost/fusion/sequence/intrinsic/begin.hpp>
#include <boost/fusion/sequence/intrinsic/end.hpp>
#include <boost/fusion/iterator/next.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/mpl/for_each.hpp>

namespace boost {
namespace fusion {

/** Create an boost::fusion::iterator_range view from the given iterators. */
template <typename B, typename E>
iterator_range<B,E> range( const B& b, const E& e )
    { return iterator_range<B,E>(b,e); }

}
}

namespace nonlinfit {
namespace sum {

/** The nonlinfit::Evaluator for a sum::Lambda. This evaluator uses the 
 *  nonlinfit::Evaluator instances (called base evaluators here) of the
 *  Lambda's parts to compute the derivatives and values.
 *
 *  \tparam The sequence of nonlinfit::Lambda types that create the 
 *          sum::Lambda.
 **/
template <typename Parts, typename ComputationTag>
class Evaluator
{
    typedef typename boost::mpl::transform<
            Parts, 
            get_evaluator< boost::mpl::_1, ComputationTag >
        >::type Evaluators;
    typename boost::fusion::result_of::as_vector< Evaluators >::type parts;
    typedef boost::mpl::range_c<int,0,boost::mpl::size<Parts>::type::value>
        Indices;
    
    template <typename Index>
    typename boost::mpl::at< Evaluators, Index >::type&
        get( Index ) { return boost::fusion::at<Index>(parts); }

    /** Free functor for the prepare_iteration method of a base evaluator.
     *  \sa nonlinfit::plane::Evaluator::prepare_iteration() */
    struct Prepare_iteration {
        typedef bool result_type;
        template <typename Evaluator, typename Data>
        bool operator()( bool last_evaluation, Evaluator& e, const Data& data ) const { 
            bool this_summand_good = e.prepare_iteration(data); 
            return last_evaluation && this_summand_good;
        }
    };
    /** Free functor for the prepare_chunk method of a base evaluator.
     *  \sa nonlinfit::plane::Evaluator::prepare_chunk() */
    struct Prepare_chunk {
        typedef void result_type;
        template <typename Evaluator, typename Target>
        void operator()( Evaluator& e, const Target& data ) const
            { e.prepare_chunk(data); }
    };
    /** Free functor for the add_value method of a base evaluator.
     *  \sa nonlinfit::plane::Evaluator::add_value() */
    struct AddValue {
        typedef void result_type;
        template <typename Evaluator, typename Target>
        void operator()( Evaluator& e, Target& result ) const 
            { e.add_value( result ); }
    };

  public:
    Evaluator( const Lambda<Parts>& expression ) 
        : parts( expression.parts ) {}

    template <typename Data>
    bool prepare_iteration( const Data& data ) {
        bool all_good = fold( parts, true, boost::bind( 
            Prepare_iteration(), _1, _2, boost::ref(data) ) );
        return all_good;
    }

    template <typename Target>
    void prepare_chunk( const Target& xs ) { 
        for_each( parts, boost::bind( 
            Prepare_chunk(), _1, boost::ref(xs) ) ); 
    }

    template <typename Result>
    void value( Result& result ) {
        /* We treat one summand specially here to avoid zero-initialization. */
        boost::fusion::at_c<0>(parts).value( result );
        for_each( range( next(begin(parts)), end(parts) ),
                  boost::bind( AddValue(), _1, boost::ref(result) ) ); 
    }

    template <typename DerivSum>
    struct reduce {
        typedef DerivationSummand< 
            typename DerivSum::Summand, 
            typename DerivSum::Parameter::base, 
            typename DerivSum::Dimension > type;
    };
    template <typename Summand>
    struct is_always_zero {
        typedef typename Summand::Parameter Parameter;
        typedef typename boost::mpl::at< Evaluators, typename Parameter::Function >::type
            ::template is_always_zero< typename reduce<Summand>::type >::type
            type;
    };

    template <typename Function, typename Target, typename BaseParam>
    void derivative( Target target, TermParameter<Function,BaseParam> ) { 
        get( Function() ).derivative( target, BaseParam() ); 
    }

    template <typename Target, typename Summand>
    void derivative( Target target, Summand )
    { 
        get( typename Summand::Parameter::Function() ).derivative( 
            target, 
            typename reduce<Summand>::type()
        ); 
    }
};

}

template <typename Parts, typename Tag>
struct get_evaluator< sum::Lambda<Parts>, Tag >
{
    typedef sum::Evaluator<Parts,Tag> type;
};

}

#endif
