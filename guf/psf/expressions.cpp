#include "debug.h"
#include "expressions.h"
#include <boost/units/Eigen/Array>
#include "BaseEvaluator.h"

namespace dStorm {
namespace guf {
namespace PSF {

BaseExpression::~BaseExpression() {}

bool BaseExpression::form_parameters_are_sane() const {
    return (best_sigma.array() >= 0).all() 
        && amplitude >= 0 && transmission >= 0 && wavelength >= 0;
}
bool Zhuang::form_parameters_are_sane() const {
    return BaseExpression::form_parameters_are_sane() && (delta_sigma.array() >= 0).all();
}

bool BaseExpression::mean_within_range( const Bound& lower, const Bound& upper ) const {
    DEBUG( "Spatial mean " << spatial_mean.transpose() << " checked to be within " << lower.transpose() << " and " << upper.transpose() );
    return (spatial_mean.array() >= boost::units::value(lower.head<2>()).array()).all()
        && (spatial_mean.array() <= boost::units::value(upper.head<2>()).array()).all();
}

bool BaseExpression::sigma_is_negligible( quantity<PixelSizeUnit> pixel_size ) const {
    return ( pixel_size.value() / (best_sigma[0] * best_sigma[1]) ) > 10;
}

Eigen::Matrix< quantity<LengthUnit>, 2, 1 > No3D::get_sigma() const
{
    return boost::units::from_value< LengthUnit >( 
        BaseEvaluator<double,No3D>(*this).get_sigma() );
}
Eigen::Matrix< quantity<LengthUnit>, 2, 1 > Zhuang::get_sigma() const
{
    return boost::units::from_value< LengthUnit >( 
        BaseEvaluator<double,Zhuang>(*this).get_sigma() );
}

}
}
}
