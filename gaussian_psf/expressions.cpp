#include "debug.h"
#include "expressions.h"
#include "BaseEvaluator.h"

namespace dStorm {
namespace gaussian_psf {

BaseExpression::BaseExpression() 
: may_leave_roi( false ) {}
BaseExpression::~BaseExpression() {}

bool BaseExpression::form_parameters_are_sane() const {
    bool is_good = amplitude >= 0 && transmission >= 0;
    if ( ! is_good ) {
        DEBUG("The form parameters " << best_sigma.transpose() << " " 
              << amplitude << " " << transmission << " are insane");
    }
    return is_good;
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
