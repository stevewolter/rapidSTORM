#ifndef NONLINFIT_ZERO_H
#define NONLINFIT_ZERO_H

#include <Eigen/Core>
#include <boost/mpl/vector.hpp>
#include <nonlinfit/Lambda.h>
#include <nonlinfit/Evaluator.h>
#include <boost/units/systems/camera/intensity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/area.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/power10.hpp>

namespace nonlinfit {
/** This namespace contains a Lambda for a function that is always zero. */
namespace zero {

template <typename Num, int ChunkSize>
class Computation;

struct Expression
{
    typedef boost::mpl::vector<> Variables;
};

template <typename Num, int ChunkSize>
struct Computation {
    Computation(const Expression&) {}
    template <typename Data>
    bool prepare_iteration( const Data& ) { return true; }
    template <typename Data> void prepare_chunk( const Data& ) {}

    void value( Eigen::Matrix<Num,ChunkSize,1>& result ) { result.fill(0); }
    void add_value( Eigen::Matrix<Num,ChunkSize,1>& ) {}
};

}

/** \cond */
template <typename Tag>
struct get_evaluator< zero::Expression, Tag > { 
    typedef zero::Computation< typename Tag::Number, Tag::ChunkSize > type; 
};
/** \endcond */

}

#endif
