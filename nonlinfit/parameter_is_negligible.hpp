#ifndef NONLINFIT_PARAMETER_IS_NEGLIGIBLE_HPP
#define NONLINFIT_PARAMETER_IS_NEGLIGIBLE_HPP

#include <boost/bind/bind.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/at.hpp>

/** Functor for checking a single variable 
 *  Gets an instance of boost::mpl::int_ and writes the corresponding 
 *  scalar in the Position to the unitless value of the indexed
 *  parameter in the Lambda. */
struct parameter_is_negligible {
    typedef void result_type;
    template <typename Expression, typename ParameterVector, typename Index>
    void operator()(const Expression& expression,
                    const ParameterVector& from,
                    const ParameterVector& to,
                    bool& target,
                    Index index) {
        typedef typename boost::mpl::at<typename Expression::Variables, Index>::type Parameter;
        target = target && expression.change_is_negligible(
                Parameter(), from[Index::value], to[Index::value]);
    }

    template <typename Expression, typename ParameterVector>
    bool all(const Expression& expression,
             const ParameterVector& old_position,
             const ParameterVector& new_position) {
        bool all_are_negligible = true;
        static const int VariableCount = boost::mpl::size<typename Expression::Variables>::value;
        boost::mpl::for_each< boost::mpl::range_c<int,0,VariableCount> >( 
            boost::bind(*this, boost::cref(expression),
                boost::cref(old_position), boost::cref(new_position),
                boost::ref(all_are_negligible), boost::placeholders::_1));
        return all_are_negligible;
    }
};


#endif
