#ifndef DSTORM_GUF_PLANEFUNCTION_HPP
#define DSTORM_GUF_PLANEFUNCTION_HPP

#include "guf/PlaneFunction.h"
#include "nonlinfit/plane/Distance.hpp"
#include "nonlinfit/plane/JointTermImplementation.h"
#include "nonlinfit/plane/DisjointTermImplementation.h"
#include "nonlinfit/FunctionConverter.h"
#include "fit_window/chunkify.hpp"

namespace dStorm {
namespace guf {

template <typename Lambda, typename Tag>
std::unique_ptr<nonlinfit::plane::Term<Tag>> create_term(Lambda& expression, Tag) {
    return std::unique_ptr<nonlinfit::plane::Term<Tag>>(new nonlinfit::plane::JointTermImplementation<Lambda, Tag>(expression));
}

template <typename Lambda, typename Num, int ChunkSize, typename OuterParam, typename InnerParam>
std::unique_ptr<nonlinfit::plane::Term<nonlinfit::plane::Disjoint<Num, ChunkSize, OuterParam, InnerParam>>> create_term(
        Lambda& expression, nonlinfit::plane::Disjoint<Num, ChunkSize, OuterParam, InnerParam>) {
    return std::unique_ptr<nonlinfit::plane::Term<nonlinfit::plane::Disjoint<Num, ChunkSize, OuterParam, InnerParam>>>(
            new nonlinfit::plane::DisjointTermImplementation<Lambda, nonlinfit::plane::Disjoint<Num, ChunkSize, OuterParam, InnerParam>>(expression));
}

template <class Lambda, class Tag, class DistanceMetric>
struct PlaneFunctionImplementation 
: public nonlinfit::AbstractFunction<double>
{
    std::unique_ptr<nonlinfit::plane::Term<Tag>> implementation;
    nonlinfit::plane::Distance< Tag, DistanceMetric > unconverted;
    nonlinfit::FunctionConverter<double, typename Tag::Number> converted;
    typename Tag::Data xs;
    std::vector<nonlinfit::DataChunk<typename Tag::Number, Tag::ChunkSize>> ys;

  public:
    PlaneFunctionImplementation( Lambda& expression, const fit_window::Plane& plane )
        : implementation(create_term(expression, Tag())), unconverted(implementation.get()), converted(unconverted) {
        fit_window::chunkify(plane, xs);
        fit_window::chunkify_data_chunks(plane, ys);
        unconverted.set_data(xs, ys);
    }

    int variable_count() const OVERRIDE { return converted.variable_count(); }
    bool evaluate( Derivatives& p ) OVERRIDE { return converted.evaluate(p); }
    void get_position( Position& p ) const OVERRIDE { converted.get_position(p); }
    void set_position( const Position& p ) OVERRIDE { converted.set_position(p); }
    bool step_is_negligible( const Position& old_position,
                             const Position& new_position ) const {
        return converted.step_is_negligible(old_position, new_position);
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <class Function, typename ComputationWay>
std::auto_ptr< nonlinfit::AbstractFunction<double> >
PlaneFunction::create( Function& e, const fit_window::Plane& data, bool mle )
{
    if (mle) {
        return std::auto_ptr< nonlinfit::AbstractFunction<double> >( 
            new PlaneFunctionImplementation<Function, ComputationWay, nonlinfit::plane::negative_poisson_likelihood>(e, data) );
    } else {
        return std::auto_ptr< nonlinfit::AbstractFunction<double> >( 
            new PlaneFunctionImplementation<Function, ComputationWay, nonlinfit::plane::squared_deviations>(e, data) );
    }
}

}
}

#endif
