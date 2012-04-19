#ifndef PSF_REFERENCEEVALUATION_H
#define PSF_REFERENCEEVALUATION_H

#include <nonlinfit/plane/fwd.h>
#include "Polynomial3D.h"
#include "No3D.h"
#include "Spline3D.h"
#include <nonlinfit/plane/GenericData.h>
#include <nonlinfit/Evaluator.h>
#include <boost/units/systems/si/area.hpp>
#include <boost/units/cmath.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include <dStorm/threed_info/DepthInfo.h>

namespace dStorm {
namespace guf {
namespace PSF {

template <typename Model, typename Number, typename P1, typename P2>
class ReferenceEvaluator ;

struct set_delta_sigma {
    typedef void result_type;
    template <typename Number, typename Index>
    void operator()( Number* x, Number* y, const Polynomial3D& expr, Index ) {
        x[Index::value] = expr( DeltaSigma<Direction_X,Index::value>() ).value();
        y[Index::value] = expr( DeltaSigma<Direction_Y,Index::value>() ).value();
    }
};

template <typename Number, typename P1, typename P2>
class ReferenceEvaluator <Polynomial3D, Number, P1, P2>
{
    Polynomial3D *expr;
    Number x, y, x0, y0, s0x, s0y;
    Number A, theta;
    Number pixelarea;
    Number zx, zy, z0, dzx[Polynomial3D::Order+1], dzy[Polynomial3D::Order+1];
    static const Number Pi = M_PI;
  public:
    ReferenceEvaluator( Polynomial3D& expr ) { this->expr = &expr; }
    typedef nonlinfit::plane::GenericData< LengthUnit > Data;
    bool prepare_iteration( const Data& data ) {
        pixelarea = data.pixel_size.value();
        return true;
    }

    void prepare_chunk( const Eigen::Array<Number,1,2>& xs ) {
        (*expr)( P1() ).set_value( xs[0] );
        (*expr)( P2() ).set_value( xs[1] );
        x = (*expr)( nonlinfit::Xs<0,LengthUnit>() ).value();
        y = (*expr)( nonlinfit::Xs<1,LengthUnit>() ).value();
        x0 = (*expr)( PSF::Mean<0>() ).value();
        y0 = (*expr)( PSF::Mean<1>() ).value();
        s0x = (*expr)( PSF::BestSigma<0>() ).value();
        s0y = (*expr)( PSF::BestSigma<1>() ).value();
        A = (*expr)( PSF::Amplitude() ).value();
        theta = (*expr)( PSF::Prefactor() ).value();
        zx = (*expr)( ZPosition<0>() ).value();
        zy = (*expr)( ZPosition<1>() ).value();
        z0 = (*expr)( MeanZ() ).value();
        boost::mpl::for_each< boost::mpl::range_c<int,1,polynomial_3d::Order+1> >
            ( boost::bind( set_delta_sigma(), dzx,dzy, *expr, _1) );
    }
    void value( Eigen::Array<Number,1,1>& result ) 
        { result.fill(0); add_value(result); }

#include "polynomial_psf_generated_by_yacas.h"

};

template <typename Number, typename P1, typename P2>
class ReferenceEvaluator <No3D, Number, P1, P2>
{
    No3D * const expr;
    static const Number Pi = M_PI;
    Number x, y, x0, y0, s0x, s0y, A, pf;
    Number pixel_size;
  public:
    typedef nonlinfit::plane::GenericData< LengthUnit > Data;
    bool prepare_iteration( const Data& data ) {
        pixel_size = data.pixel_size.value(); 
        return true;
    }

    ReferenceEvaluator( No3D& expr ) : expr(&expr) {}
    void prepare_chunk( const Eigen::Array<Number,1,2>& xs ) {
        (*expr)( P1() ).set_value( xs[0] );
        (*expr)( P2() ).set_value( xs[1] );
        x = (*expr)( nonlinfit::Xs<0,LengthUnit>() ).value();
        y = (*expr)( nonlinfit::Xs<1,LengthUnit>() ).value();
        x0 = (*expr)( PSF::Mean<0>() ).value();
        y0 = (*expr)( PSF::Mean<1>() ).value();
        s0x = (*expr)( PSF::BestSigma<0>() ).value();
        s0y = (*expr)( PSF::BestSigma<1>() ).value();
        A = (*expr)( PSF::Amplitude() ).value();
        pf = (*expr)( PSF::Prefactor() ).value();
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
    void derivative( Target target, nonlinfit::Xs<Dim,LengthUnit> ) {
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
class ReferenceEvaluator <Spline3D, Number, P1, P2>
{
    Spline3D * const expr;
    static const Number Pi = M_PI;
    Number x, y, x0, y0, s0x, s0y, A, pf;
    Number dsx, dsy;
    Number pixel_size;
  public:
    typedef nonlinfit::plane::GenericData< LengthUnit > Data;
    bool prepare_iteration( const Data& data ) {
        pixel_size = data.pixel_size.value(); 
        return true;
    }

    ReferenceEvaluator( Spline3D& expr ) : expr(&expr) {}
    void prepare_chunk( const Eigen::Array<Number,1,2>& xs ) {
        (*expr)( P1() ).set_value( xs[0] );
        (*expr)( P2() ).set_value( xs[1] );
        x = (*expr)( nonlinfit::Xs<0,LengthUnit>() ).value();
        y = (*expr)( nonlinfit::Xs<1,LengthUnit>() ).value();
        x0 = (*expr)( PSF::Mean<0>() ).value();
        y0 = (*expr)( PSF::Mean<1>() ).value();
        s0x = expr->get_sigma().x().value();
        s0y = expr->get_sigma().y().value();
        A = (*expr)( PSF::Amplitude() ).value();
        pf = (*expr)( PSF::Prefactor() ).value();
        threed_info::ZPosition z( *(*expr)( MeanZ() ) );
        dsx = expr->get_spline(Direction_X).get_sigma_deriv(z );
        dsy = expr->get_spline(Direction_Y).get_sigma_deriv(z );
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
    void derivative( Target target, nonlinfit::Xs<Dim,LengthUnit> ) {
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
}

#endif
