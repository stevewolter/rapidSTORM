#ifndef FITTER_CONSTANT_BACKGROUND_HPP
#define FITTER_CONSTANT_BACKGROUND_HPP

#include "constant_background/fwd.hpp"
#include <Eigen/Core>
#include <boost/mpl/vector.hpp>
#include <nonlinfit/access_parameters.hpp>
#include <nonlinfit/DerivationSummand.h>
#include <nonlinfit/Evaluator.h>
#include <nonlinfit/plane/fwd.h>

namespace dStorm {
namespace constant_background {

inline std::ostream& operator<<(std::ostream& o, Amount)  { return (o << "constant"); }

/** This class models a globally constant function. 
 *  Evaluating this function at any point will yield the same value. */
struct Expression
: public nonlinfit::access_parameters< Expression >
{
    typedef boost::mpl::vector< Amount > Variables;
    Expression() : relative_epsilon(0) {}

    bool change_is_negligible(Amount tag, double from, double to) const {
        double rel_change = std::abs(to - from) / std::max(1E-10, std::max(std::abs(from), std::abs(to)));
        return rel_change < relative_epsilon;
    }

    void set_relative_epsilon(double relative_epsilon) {
        this->relative_epsilon = relative_epsilon;
    }

  private:
    double amount;
    double relative_epsilon;
    double& access( Amount ) { return amount; }
    friend class access_parameters< Expression >;
};

template <typename Num, typename Param>
class Computation {
    const Expression * const m;
    Num a;
  public:
    Computation( const Expression& m ) : m(&m) {}
    template <typename Data>
    bool prepare_iteration( const Data& ) { 
        a = (*m)( Amount() ); 
        return a >= 0; 
    }
    template <typename Data> void prepare_chunk( const Data& ) {}

    template <typename DerivationSummand>
    struct is_always_zero {
        typedef boost::mpl::bool_< DerivationSummand::Summand::value != 0 > 
            type;
    };
    template <typename Target>
    void value( Target& result ) { result.fill(a); }
    template <typename Target>
    void add_value( Target& result ) { result.array() += a; }

    template <typename Target>
    void derivative( Target target, const Amount& ) { target.fill(1); }
    template <typename Target, class Dimension>
    void derivative( Target target, const nonlinfit::DerivationSummand< Param, Amount, Dimension>& ) { target.fill(1); }
    template <typename Target, class Dimension, typename OtherParam>
    void derivative( Target target, const nonlinfit::DerivationSummand< OtherParam, Amount, Dimension>& ) { target.fill(0); }
};

}
}

namespace nonlinfit {

/** \cond */
template <typename Num, int ChunkSize, typename P1, typename P2>
struct get_evaluator< dStorm::constant_background::Expression, plane::Disjoint<Num,ChunkSize,P1,P2> >
    { typedef dStorm::constant_background::Computation<Num,P1> type; };
template <typename Num, int ChunkSize, typename P1, typename P2>
struct get_evaluator< dStorm::constant_background::Expression, plane::Joint<Num,ChunkSize,P1,P2> >
    { typedef dStorm::constant_background::Computation<Num,P1> type; };
/** \endcond */

}

#endif
