#include <boost/math/constants/constants.hpp>

#include "gaussian_psf/mock_model.h"
#include <boost/mpl/for_each.hpp>
#include <Eigen/StdVector>
#include "gaussian_psf/parameters.h"
#include "gaussian_psf/No3D.h"
#include "gaussian_psf/DepthInfo3D.h"
#include <boost/smart_ptr/make_shared.hpp>
#include "threed_info/Spline3D.h"

namespace dStorm {
namespace gaussian_psf {

fit_window::Plane
mock_data() {
    fit_window::Plane result;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 12; ++j) {
            fit_window::DataPoint point;
            point.position.x() = j * exp(1) / 30.0;
            point.position.y() = i * boost::math::constants::pi<double>() / 30.0;
            point.value = rand() * 1E-5;
            result.points.push_back( point );
        }
    }
    result.min_coordinate[0] = -1E50;
    result.min_coordinate[1] = -1E50;
    result.max_coordinate[0] = 1E50;
    result.max_coordinate[1] = 1E50;
    result.pixel_size = 1E-3;
    result.window_width = 12;
    return result;
}

template <typename Expression>
struct RandomParameterSetter {
    Expression m;
    typedef void result_type;

    RandomParameterSetter() {}

    template <int Dim>
    void operator()( BestSigma<Dim> p ) { m(p) = 0.400+ 0.1*Dim; }
    template <int Dim>
    void operator()( nonlinfit::Xs<Dim> ) {}
    template <int Dim>
    void operator()( Mean<Dim> p ) { m(p) = 2.500+ 0.050*Dim; }
    void operator()( MeanZ p ) { m(p) = 0.600; }
    template <int Dim>
    void operator()( ZPosition<Dim> p ) { m(p) = 0.200- 0.050*Dim; }
    void operator()( Amplitude p ) { m(p) = 1E7; }
    template <int Dim, int Term>
    void operator()( DeltaSigma<Dim,Term> p ) { m(p) = 1.6 - 0.4 * Dim - 0.1 * Term; }
    void operator()( Prefactor p ) { m(p) = 0.3; }

    const Expression& operator()() { 
        boost::mpl::for_each< typename Expression::Variables >( boost::ref(*this) );
        return m;
    }
};

template <typename Expression> 
Expression mock_model() { return RandomParameterSetter<Expression>()(); }

template No3D mock_model<No3D>();
template <>
DepthInfo3D mock_model<DepthInfo3D>() {
    DepthInfo3D result = RandomParameterSetter<DepthInfo3D>()();
    result.set_spline( 
        boost::make_shared< threed_info::Spline3D>( threed_info::SplineFactory::Mock(Direction_X) ),
        boost::make_shared< threed_info::Spline3D>( threed_info::SplineFactory::Mock(Direction_Y) )
    );
    return result;
}

}
}
