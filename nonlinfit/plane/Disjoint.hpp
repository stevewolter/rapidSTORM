#ifndef NONLINFIT_PLANE_DISJOINT_HPP
#define NONLINFIT_PLANE_DISJOINT_HPP

#include "nonlinfit/plane/fwd.h"
#include "nonlinfit/plane/Disjoint.h"
#include <nonlinfit/DerivationSummand.h>
#include <nonlinfit/Evaluator.h>
#include <boost/bind/bind.hpp>
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
class MatrixReducer
{
    template <typename ReducerTag>
    struct fill_array {
        typedef void result_type;
        template <typename Target, typename Term>
        void operator()( Target& a, Term ) {
            const int from = index_of< typename ReducerTag::TermVariables, Term >::value;
            const int to = index_of< typename ReducerTag::OutputVariables, typename Term::Parameter >::value;
            a[from] = to;
        }
    };

    template <bool Rows, bool Columns, typename Target, typename Source, typename Reducer>
    static void do_combine( Target& t, const Source& s, const Reducer& a) {
        t.fill(0);
        for (int r = 0; r < s.rows(); ++r)
            for (int c = 0; c < s.cols(); ++c)
                t( ( Rows ) ? a[r] : r, ( Columns ) ? a[c] : c ) += s(r,c);
    }
  public:
    template <typename ReducerTag>
    static Eigen::Matrix<int, ReducerTag::TermCount, 1> create_reduction_list() {
        Eigen::Matrix<int, ReducerTag::TermCount, 1> result;
        boost::mpl::for_each<typename ReducerTag::TermVariables>( boost::bind(
                    fill_array<ReducerTag>(), boost::ref(result), _1) ); 
        return result;
    }

    /** Apply the reduction to both dimensions (rows and columns). */
    template <typename Target, typename Source, typename Reducer>
    static void matrix( Target& t, const Source& s, const Reducer& r) { do_combine<true,true>(t,s,r); }
    /** Reduce only the rows, leaving the column count constant. */
    template <typename Target, typename Source, typename Reducer>
    static void vector( Target& t, const Source& s, const Reducer& r) { do_combine<true,false>(t,s,r); }
    /** Reduce only the columns, leaving the row count constant. */
    template <typename Target, typename Source, typename Reducer>
    static void row_vector( Target& t, const Source& s, const Reducer& r) { do_combine<false,true>(t,s,r); }
};

template <typename DataTag, typename Lambda>
struct ReducerTag {
    typedef typename DataTag::template make_derivative_terms<Lambda,void>::type TermVariables;
    typedef typename Lambda::Variables OutputVariables;
    static const int TermCount = boost::mpl::size<TermVariables>::type::value;
};

}
}

#endif
