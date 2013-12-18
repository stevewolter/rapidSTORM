#ifndef PSF_REFERENCEEVALUATION_H
#define PSF_REFERENCEEVALUATION_H

#include <boost/math/constants/constants.hpp>

#include <nonlinfit/plane/fwd.h>
#include "No3D.h"
#include "DepthInfo3D.h"
#include <nonlinfit/plane/GenericData.h>
#include <nonlinfit/Evaluator.h>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include <dStorm/threed_info/DepthInfo.h>

namespace dStorm {
namespace gaussian_psf {

template <typename Model, typename Number, typename P1, typename P2>
class ReferenceEvaluator ;

template <typename Number, typename P1, typename P2>
class ReferenceEvaluator <No3D, Number, P1, P2>
{
    No3D * const expr;
    static constexpr Number Pi = boost::math::constants::pi<double>();
    Number x, y, x0, y0, s0x, s0y, A, pf;
    Number pixel_size;
  public:
    bool prepare_iteration( const nonlinfit::plane::GenericData& data ) {
        pixel_size = data.pixel_size; 
        return true;
    }

    ReferenceEvaluator( No3D& expr ) : expr(&expr) {}
    void prepare_chunk( const Eigen::Array<Number,1,2>& xs ) {
        (*expr)( P1() ) = xs[0];
        (*expr)( P2() ) = xs[1];
        x = (*expr)( nonlinfit::Xs<0>() );
        y = (*expr)( nonlinfit::Xs<1>() );
        x0 = (*expr)( Mean<0>() );
        y0 = (*expr)( Mean<1>() );
        s0x = (*expr)( BestSigma<0>() );
        s0y = (*expr)( BestSigma<1>() );
        A = (*expr)( Amplitude() );
        pf = (*expr)( Prefactor() );
    }
    void value( Eigen::Array<Number,1,1>& result ) 
        { result.fill(0); add_value(result); }
    void add_value( Eigen::Array<Number,1,1>& result ) { 
        result(0,0) += ( exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  *
          1.0, 2) + pow(y - y0, 2) / pow(s0y  * 1.0, 2)) ) * A *
          pf * pixel_size)  / ( 2 * Pi * s0x  * 1.0 * s0y  * 1.0) ;
    }

    template <typename Target>
    void derivative( Target t, Amplitude ) {
        t(0,0) = ( exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0, 2)
        + pow(y - y0, 2) / pow(s0y  * 1.0, 2)) ) * pf * pixel_size)  /
        ( 2 * Pi * s0x  * 1.0 * s0y  * 1.0) ;
    }

    template <typename Target>
    void derivative( Target t, Prefactor ) {
        t(0,0) = ( exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0, 2)
        + pow(y - y0, 2) / pow(s0y  * 1.0, 2)) ) * A * pixel_size)  /
        ( 2 * Pi * s0x  * 1.0 * s0y  * 1.0) ;
    }

    template <typename Target>
    void derivative( Target t, Mean<0>  ) {
        t(0,0) = (  - ( A * pf * pixel_size * exp( - 0.5 * ( pow(x -
        x0, 2) / pow(s0x  * 1.0, 2) + pow(y - y0, 2) / pow(s0y  *
        1.0, 2)) ) * ( x0 - x) )  / pow(s0x  * 1.0, 2))  / ( 2 *
        Pi * s0x  * 1.0 * s0y  * 1.0) ;
    }

    template <typename Target>
    void derivative( Target t, Mean<1>  ) {
        t(0,0) = (  - ( A * pf * pixel_size * exp( - 0.5 * ( pow(x -
        x0, 2) / pow(s0x  * 1.0, 2) + pow(y - y0, 2) / pow(s0y  *
        1.0, 2)) ) * ( y0 - y) )  / pow(s0y  * 1.0, 2))  / ( 2 *
        Pi * s0x  * 1.0 * s0y  * 1.0) ;
    }

    template <typename Target, int Dim>
    void derivative( Target target, nonlinfit::Xs<Dim> ) {
        derivative( target, Mean<Dim>() );
        target(0,0) *= -1;
    }

    template <typename Target>
    void derivative( Target t, BestSigma<0>  ) {
        t(0,0) = (  - ( ( 2 * Pi * s0x  * 1.0 * s0y  * 1.0 * A * pf
        * pixel_size * exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0,
        2) + pow(y - y0, 2) / pow(s0y  * 1.0, 2)) ) * -0.5 * pow(x -
        x0, 2) * 2 * 1.0 * s0x  * 1.0)  / pow(s0x  * 1.0, 4) +
        exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0, 2) + pow(y -
        y0, 2) / pow(s0y  * 1.0, 2)) ) * A * pf * pixel_size * 2 *
        Pi * 1.0 * s0y  * 1.0) )  / pow(2 * Pi * s0x  * 1.0 *
        s0y  * 1.0, 2);
    }

    template <typename Target>
    void derivative( Target t, BestSigma<1>  ) {
        t(0,0) = (  - ( ( 2 * Pi * s0x  * 1.0 * s0y  * 1.0 * A * pf
        * pixel_size * exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0,
        2) + pow(y - y0, 2) / pow(s0y  * 1.0, 2)) ) * -0.5 * pow(y -
        y0, 2) * 2 * 1.0 * s0y  * 1.0)  / pow(s0y  * 1.0, 4) +
        exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0, 2) + pow(y -
        y0, 2) / pow(s0y  * 1.0, 2)) ) * A * pf * pixel_size * 2 *
        Pi * s0x  * 1.0 * 1.0) )  / pow(2 * Pi * s0x  * 1.0 *
        s0y  * 1.0, 2);
    }

};

template <typename Number, typename P1, typename P2>
class ReferenceEvaluator <DepthInfo3D, Number, P1, P2>
{
    DepthInfo3D * const expr;
    static constexpr Number Pi = boost::math::constants::pi<double>();
    Number x, y, x0, y0, s0x, s0y, A, pf;
    Number dsx, dsy;
    Number pixel_size;
  public:
    bool prepare_iteration( const nonlinfit::plane::GenericData& data ) {
        pixel_size = data.pixel_size; 
        return true;
    }

    ReferenceEvaluator( DepthInfo3D& expr ) : expr(&expr) {}
    void prepare_chunk( const Eigen::Array<Number,1,2>& xs ) {
        (*expr)( P1() ) = xs[0];
        (*expr)( P2() ) = xs[1];
        x = (*expr)( nonlinfit::Xs<0>() );
        y = (*expr)( nonlinfit::Xs<1>() );
        x0 = (*expr)( Mean<0>() );
        y0 = (*expr)( Mean<1>() );
        s0x = expr->get_sigma().x();
        s0y = expr->get_sigma().y();
        A = (*expr)( Amplitude() );
        pf = (*expr)( Prefactor() );
        threed_info::ZPosition z( (*expr)( MeanZ() ) );
        dsx = expr->get_spline(Direction_X).get_sigma_deriv(z);
        dsy = expr->get_spline(Direction_Y).get_sigma_deriv(z);
    }
    void value( Eigen::Array<Number,1,1>& result ) 
        { result.fill(0); add_value(result); }
    void add_value( Eigen::Array<Number,1,1>& result ) { 
        result(0,0) += ( exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  *
          1.0, 2) + pow(y - y0, 2) / pow(s0y  * 1.0, 2)) ) * A *
          pf * pixel_size)  / ( 2 * Pi * s0x  * 1.0 * s0y  * 1.0) ;
    }

    template <typename Target>
    void derivative( Target t, Amplitude ) {
        t(0,0) = ( exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0, 2)
        + pow(y - y0, 2) / pow(s0y  * 1.0, 2)) ) * pf * pixel_size)  /
        ( 2 * Pi * s0x  * 1.0 * s0y  * 1.0) ;
    }

    template <typename Target>
    void derivative( Target t, Prefactor ) {
        t(0,0) = ( exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0, 2)
        + pow(y - y0, 2) / pow(s0y  * 1.0, 2)) ) * A * pixel_size)  /
        ( 2 * Pi * s0x  * 1.0 * s0y  * 1.0) ;
    }

    template <typename Target>
    void derivative( Target t, Mean<0>  ) {
        t(0,0) = (  - ( A * pf * pixel_size * exp( - 0.5 * ( pow(x -
        x0, 2) / pow(s0x  * 1.0, 2) + pow(y - y0, 2) / pow(s0y  *
        1.0, 2)) ) * ( x0 - x) )  / pow(s0x  * 1.0, 2))  / ( 2 *
        Pi * s0x  * 1.0 * s0y  * 1.0) ;
    }

    template <typename Target>
    void derivative( Target t, Mean<1>  ) {
        t(0,0) = (  - ( A * pf * pixel_size * exp( - 0.5 * ( pow(x -
        x0, 2) / pow(s0x  * 1.0, 2) + pow(y - y0, 2) / pow(s0y  *
        1.0, 2)) ) * ( y0 - y) )  / pow(s0y  * 1.0, 2))  / ( 2 *
        Pi * s0x  * 1.0 * s0y  * 1.0) ;
    }

    template <typename Target, int Dim>
    void derivative( Target target, nonlinfit::Xs<Dim> ) {
        derivative( target, Mean<Dim>() );
        target(0,0) *= -1;
    }

    template <typename Target>
    void derivative( Target t, MeanZ  ) {
        Number dPSF_over_dsigma_x = ( (  - ( ( 2 * Pi * s0x  * 1.0 * s0y  * 1.0 * A * pf
            * pixel_size * exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0,
            2) + pow(y - y0, 2) / pow(s0y  * 1.0, 2)) ) * -0.5 * pow(x -
            x0, 2) * 2 * 1.0 * s0x  * 1.0)  / pow(s0x  * 1.0, 4) +
            exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0, 2) + pow(y -
            y0, 2) / pow(s0y  * 1.0, 2)) ) * A * pf * pixel_size * 2 *
            Pi * 1.0 * s0y  * 1.0) )  / pow(2 * Pi * s0x  * 1.0 *
            s0y  * 1.0, 2) );
        Number dPSF_over_dsigma_y = (  - ( ( 2 * Pi * s0x  * 1.0 * s0y  * 1.0 * A * pf
            * pixel_size * exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0,
            2) + pow(y - y0, 2) / pow(s0y  * 1.0, 2)) ) * -0.5 * pow(y -
            y0, 2) * 2 * 1.0 * s0y  * 1.0)  / pow(s0y  * 1.0, 4) +
            exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * 1.0, 2) + pow(y -
            y0, 2) / pow(s0y  * 1.0, 2)) ) * A * pf * pixel_size * 2 *
            Pi * s0x  * 1.0 * 1.0) )  / pow(2 * Pi * s0x  * 1.0 *
            s0y  * 1.0, 2);
        t(0,0) = dsx * dPSF_over_dsigma_x + dsy * dPSF_over_dsigma_y;
    }
};

}
}

#endif
