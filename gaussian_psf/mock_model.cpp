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

MockDataTag::Data
mock_data() {
    MockDataTag::Data disjoint_data;
    for (int i = 0; i < 10; ++i) {
        disjoint_data.data.push_back( MockDataTag::Data::DataRow() );
        MockDataTag::Data::DataRow& d = disjoint_data.data.back();
        d.inputs(0,0) = i * boost::math::constants::pi<double>() / 30.0;
        for (int j = 0; j < 12; ++j) {
            d.output[j] = rand() * 1E-5;
            d.logoutput[j] = (d.output[j] < 1E-10)
                ? -23*d.output[j] : d.output[j] * log(d.output[j]);
        }
    }
    disjoint_data.min.fill( -1E50 );
    disjoint_data.max.fill( 1E50 );
    disjoint_data.pixel_size = 1E-3;
    for (int j = 0; j < 12; ++j)
        disjoint_data.xs[j] = j * exp(1) / 30.0;
    return disjoint_data;
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
