#ifndef NONLINFIT_CONSTANT_H
#define NONLINFIT_CONSTANT_H

#include <Eigen/Core>
#include <boost/mpl/vector.hpp>
#include <nonlinfit/access_parameters.hpp>
#include <nonlinfit/DerivationSummand.h>
#include <nonlinfit/Evaluator.h>

namespace nonlinfit {
/** Contains lambda, parameters and evaluators for a constant function. */
namespace constant {

struct Amount {};

inline std::ostream& operator<<(std::ostream& o, Amount)  { return (o << "constant"); }

template <typename Num, int ChunkSize>
class Computation;

/** This class models a lambda for a globally constant function. 
 *  Evaluating this function at any point will yield the same value. */
struct Expression
: public access_parameters< Expression >
{
    typedef boost::mpl::vector< Amount > Variables;

  private:
    double amount;
    double& access( Amount ) { return amount; }
    friend class access_parameters< Expression >;
    template <class N, int C> friend class Computation;
};

template <typename Num, int ChunkSize>
class Computation {
    const Expression * const m;
  public:
    Computation(const Expression& m) : m(&m) {}
    template <typename Data>
    bool prepare_iteration( const Data& ) { return true; }
    template <typename Data> void prepare_chunk( const Data& ) {}

    void value( Eigen::Matrix<Num,ChunkSize,1>& result ) { result.fill(m->amount); }
    void add_value( Eigen::Matrix<Num,ChunkSize,1>& result ) 
        { result.array() += m->amount; }

    template <typename Target>
    void derivative( Target target, const Amount& ) { target.fill(1); }
    template <typename Target, class Dimension>
    void derivative( Target target, const DerivationSummand< boost::mpl::int_<0>, Amount, Dimension>& ) { target.fill(1); }
};

}

/** \cond */
template <typename Tag>
struct get_evaluator< constant::Expression, Tag >
    { typedef constant::Computation< typename Tag::Number, Tag::ChunkSize > type; };
/** \endcond */

}

#endif
