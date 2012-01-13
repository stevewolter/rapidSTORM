#ifndef NONLINFIT_PLANE_DISJOINT_HPP
#define NONLINFIT_PLANE_DISJOINT_HPP

#include "fwd.h"
#include "Disjoint.h"
#include <nonlinfit/DerivationSummand.h>
#include <nonlinfit/Evaluator.h>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/quote.hpp>
#include <boost/mpl/remove_if.hpp>
#include <boost/array.hpp>
#include <nonlinfit/append.h>
#include <nonlinfit/index_of.h>

namespace nonlinfit {
namespace plane {

template <typename Num, int _ChunkSize, typename OuterParam, typename InnerParam>
template <typename Lambda, typename DerivP>
struct Disjoint<Num,_ChunkSize,OuterParam,InnerParam>::make_derivative_terms
{
  private:
    typedef typename get_evaluator< Lambda, Disjoint >::type
        Evaluator;

    typedef typename nonlinfit::append<
        typename boost::mpl::transform<
            typename Lambda::Variables,
            DerivationSummand< OuterParam, boost::mpl::_1, DerivP >
        >::type,
        typename boost::mpl::transform<
            typename Lambda::Variables,
            DerivationSummand< InnerParam, boost::mpl::_1, DerivP >
        >::type
    >::type UnfilteredTerms;

    template <typename D>
    struct is_always_zero {
        typedef typename Evaluator::template is_always_zero<D>::type type;
    };

  public:
    typedef typename boost::mpl::remove_if<
        UnfilteredTerms,
        boost::mpl::quote1<is_always_zero> >::type
    type;
};

/** Map each row and/or column of one Eigen::Matrix to a smaller.
 *  The rows/columns of the input and output matrices represent
 *  elements of two lists, with the Function metafunction converting
 *  from InputList to OutputList elements.
 *
 *  \tparam InputList Elements of the input matrix.
 *  \tparam OutputList Elements of the output matrix.
 *  \tparam Function Boost.MPL metafunction converting the elements of
 *                   InputList into those in OutputList
 **/
template <typename InputList, typename OutputList, class Function>
class MatrixReducer
{
    typedef boost::array< int, boost::mpl::size<InputList>::type::value > Array;

    struct fill_array {
        Array& a;
        fill_array(Array& a) : a(a) {}
        template <typename Term>
        void operator()( Term ) 
        {
            const int to = index_of< 
                OutputList, typename Function::template apply<Term>::type >::value;
            a[ index_of< InputList, Term >::value ] = to;
        }
    };
    Array a;

    template <bool Rows, bool Columns, typename Target, typename Source>
    void do_combine( Target& t, const Source& s ) {
        t.fill(0);
        for (int r = 0; r < s.rows(); ++r)
            for (int c = 0; c < s.cols(); ++c)
                t( ( Rows ) ? a[r] : r, ( Columns ) ? a[c] : c ) += s(r,c);
    }
  public:
    MatrixReducer() { boost::mpl::for_each<InputList>( fill_array(a) ); }

    /** Apply the reduction to both dimensions (rows and columns). */
    template <typename Target, typename Source>
    void matrix( Target& t, const Source& s ) { do_combine<true,true>(t,s); }
    /** Reduce only the rows, leaving the column count constant. */
    template <typename Target, typename Source>
    void vector( Target& t, const Source& s ) { do_combine<true,false>(t,s); }
    /** Reduce only the columns, leaving the row count constant. */
    template <typename Target, typename Source>
    void row_vector( Target& t, const Source& s ) { do_combine<false,true>(t,s); }
};

template <typename Num, int _ChunkSize, typename OuterParam, typename InnerParam>
template <typename Lambda>
class Disjoint<Num,_ChunkSize,OuterParam,InnerParam>::get_derivative_combiner {
  public:
    typedef MatrixReducer< 
        typename make_derivative_terms<Lambda,void>::type,
        typename Lambda::Variables,
        boost::mpl::quote1<get_parameter>
    > type;
};

}
}

#endif
