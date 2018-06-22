#include "debug.h"
#include "gaussian_psf/expressions.h"
#include "gaussian_psf/BaseEvaluator.h"

namespace dStorm {
namespace gaussian_psf {

BaseExpression::BaseExpression() 
: may_leave_roi( false ), absolute_position_epsilon(0), relative_epsilon(0) {}
BaseExpression::~BaseExpression() {}

bool BaseExpression::form_parameters_are_sane() const {
    bool is_good = amplitude >= 0 && transmission >= 0;
    if ( ! is_good ) {
        DEBUG("The form parameters " << amplitude << " " << transmission << " are insane");
    }
    return is_good;
}

bool BaseExpression::relative_step_is_negligible(double from, double to) const {
    //double rel_change = std::abs(to - from) / std::max(1E-10, std::max(std::abs(from), std::abs(to)));
    double rel_change = std::abs((to - from) / to);
    return rel_change < relative_epsilon;
}

bool BaseExpression::mean_within_range( const Bound& lower, const Bound& upper ) const {
    if ( may_leave_roi ) return true;
    bool is_good = 
           (spatial_mean.array() >= lower.head<2>().array()).all()
        && (spatial_mean.array() <= upper.head<2>().array()).all();
    if ( ! is_good ) {
        DEBUG( "Spatial mean " << spatial_mean.transpose() << " is outside the boundaries " << lower.transpose() << " and " << upper.transpose() );
    }
    return is_good;
}

}
}
