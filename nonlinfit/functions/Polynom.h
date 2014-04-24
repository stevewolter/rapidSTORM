#ifndef NONLINFIT_STATIC_POWER_H
#define NONLINFIT_STATIC_POWER_H

#include <Eigen/Core>
#include <boost/mpl/vector.hpp>
#include <nonlinfit/Lambda.h>
#include <nonlinfit/Bind.h>
#include <nonlinfit/VectorPosition.hpp>
#include <nonlinfit/plane/fwd.h>
#include <nonlinfit/access_parameters.hpp>
#include "nonlinfit/AbstractFunction.h"

namespace nonlinfit {
namespace static_power {

struct Prefactor {};
struct Variable {};
struct Power {};

inline std::ostream& operator<<(std::ostream& o, Power)  { return (o << "prefactor"); }

template <int D, int Ds>
struct SimpleFunction ;

struct Expression
: public access_parameters< Expression >
{
    typedef boost::mpl::vector< Prefactor, Variable, Power > Variables;

  private:
    friend class access_parameters< Expression >;
    template <int D, int Ds> friend class SimpleFunction;
    double pre, var, power;
    double &access( Prefactor ) { return pre; }
    double &access( Variable ) { return var; }
    double &access( Power ) { return power; }
};

struct BaseValue { template <typename Type> struct apply; };

template <> struct BaseValue::apply< Prefactor > { typedef boost::mpl::false_ type; };
template <> struct BaseValue::apply< Variable > { typedef boost::mpl::true_ type; };
template <> struct BaseValue::apply< Power > { typedef boost::mpl::false_ type; };

template <int Dimension, int Dimensions>
struct SimpleFunction 
: public nonlinfit::AbstractFunction<double>
{
    typedef nonlinfit::Evaluation<double> Derivatives;
    static_power::Expression* expression;

    SimpleFunction( nonlinfit::Bind< Expression, BaseValue >& m ) 
        : expression(&m)
    {
    }
    bool evaluate( Derivatives& p ) {
        assert( expression );
        p.gradient.fill(0);
        p.hessian.fill(0);
        p.gradient[Dimension] = - this->expression->pre * pow( this->expression->var, this->expression->power - 1 );
        p.hessian(Dimension,Dimension) = p.gradient[Dimension] * p.gradient[Dimension];
        p.value = this->expression->pre * pow(this->expression->var, this->expression->power);
        return true;
    }

    static const int VariableCount = 1;
    int variable_count() const { return 1; }
    void get_position(Position& position) const { position[0] = this->expression->var; }
    void set_position(const Position& position) { this->expression->var = position[0]; }
    bool step_is_negligible(const Position& from, const Position& to) const {
        return false;
    }
};

}
}

#endif
