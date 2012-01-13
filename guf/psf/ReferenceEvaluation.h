#ifndef PSF_REFERENCEEVALUATION_H
#define PSF_REFERENCEEVALUATION_H

#include <nonlinfit/plane/fwd.h>
#include "Zhuang.h"
#include "No3D.h"
#include <nonlinfit/plane/GenericData.h>
#include <nonlinfit/Evaluator.h>

namespace dStorm {
namespace guf {
namespace PSF {

template <typename Model, typename Number, typename P1, typename P2>
class ReferenceEvaluator ;

template <typename Number, typename P1, typename P2>
class ReferenceEvaluator <Zhuang, Number, P1, P2>
{
    Zhuang *expr;
    Number x, y, x0, y0, s0x, s0y, A, pf, lambda;
    Number pixel_size;
    Number z, z0, z0x, z0y, cx, cy;
    static const Number Pi = M_PI;
  public:
    ReferenceEvaluator( Zhuang& expr ) { this->expr = &expr; }
    typedef nonlinfit::plane::GenericData< LengthUnit > Data;
    bool prepare_iteration( const Data& data ) {
        pixel_size = data.pixel_size.value();
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
        pf = (*expr)( PSF::Prefactor() ).value();
        lambda = (*expr)( PSF::Wavelength() ).value();
        z = (*expr)( ZPosition() ).value();
        z0 = (*expr)( MeanZ() ).value();
        z0x = (*expr)( ZOffset<0>() ).value();
        z0y = (*expr)( ZOffset<1>() ).value();
        cx = (*expr)( DeltaSigma<0>() ).value();
        cy = (*expr)( DeltaSigma<1>() ).value();
    }
    void value( Eigen::Array<Number,1,1>& result ) 
        { result.fill(0); add_value(result); }
    void add_value( Eigen::Array<Number,1,1>& result ) {
        result(0,0) += ( exp( - 0.5 * ( pow(x - x0, 2) / pow(( cx * pow(z
        - ( z0x + z0) , 2) + s0x)  * lambda, 2) + pow(y - y0, 2) / pow((
        cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 2)) ) * A * pf *
        pixel_size)  / ( 2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) + s0x)
        * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda) ;
    }
    template <typename Target>
    void derivative( Target target, Mean<0> ) {
        target(0,0) = (  - ( A * pf * pixel_size * exp( - 0.5 * ( pow(x -
        x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda,
        2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)
        * lambda, 2)) ) * ( x0 - x) )  / pow(( cx * pow(z - ( z0x + z0)
        , 2) + s0x)  * lambda, 2))  / ( 2 * Pi * ( cx * pow(z - ( z0x +
        z0) , 2) + s0x)  * lambda * ( cy * pow(z - ( z0y + z0) , 2) +
        s0y)  * lambda) ;
    }

    template <typename Target>
    void derivative( Target target, Mean<1> ) {
	target(0,0) = (  - ( A * pf * pixel_size * exp( - 0.5 * ( pow(x -
	x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda,
	2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)
	* lambda, 2)) ) * ( y0 - y) )  / pow(( cy * pow(z - ( z0y + z0)
	, 2) + s0y)  * lambda, 2))  / ( 2 * Pi * ( cx * pow(z - ( z0x +
	z0) , 2) + s0x)  * lambda * ( cy * pow(z - ( z0y + z0) , 2) +
	s0y)  * lambda) ;
    }

    template <typename Target, int Dim>
    void derivative( Target target, nonlinfit::Xs<Dim,LengthUnit> ) {
        derivative( target, Mean<Dim>() );
        target(0,0) *= -1;
    }

    template <typename Target>
    void derivative( Target target, MeanZ ) {
        target(0,0) = (  - ( 2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) +
        s0x)  * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda
        * 0.5 * ( (  - pow(y - y0, 2) * 2 * cy * -2 * ( z - ( z0y + z0)
        )  * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda)
        / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 4) - (
        pow(x - x0, 2) * 2 * cx * -2 * ( z - ( z0x + z0) )  * lambda *
        ( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda) / pow(( cx *
        pow(z - ( z0x + z0) , 2) + s0x)  * lambda, 4))  * exp( - 0.5 *
        ( pow(x - x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  *
        lambda, 2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0) ,
        2) + s0y)  * lambda, 2)) ) * A * pf * pixel_size + exp( - 0.5 *
        ( pow(x - x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)
        * lambda, 2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0)
        , 2) + s0y)  * lambda, 2)) ) * A * pf * pixel_size * ( 2 * Pi *
        ( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda * cy * -2 * ( z -
        ( z0y + z0) )  * lambda + 2 * Pi * cx * -2 * ( z - ( z0x + z0) )
        * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda) ) )
        / pow(2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda *
        ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 2);
    }
    template <typename Target>
    void derivative( Target target, ZPosition ) {
        derivative( target, MeanZ() );
        target *= -1;
    }

    template <typename Target>
    void derivative( Target target, BestSigma<0> ) {
	target(0,0) = (  - ( ( 2 * Pi * ( cx * pow(z - ( z0x + z0) ,
	2) + s0x)  * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)
	* lambda * A * pf * pixel_size * exp( - 0.5 * ( pow(x - x0,
	2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda, 2)
	+ pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)
	* lambda, 2)) ) * -0.5 * pow(x - x0, 2) * 2 * lambda * ( cx *
	pow(z - ( z0x + z0) , 2) + s0x)  * lambda)  / pow(( cx * pow(z -
	( z0x + z0) , 2) + s0x)  * lambda, 4) + exp( - 0.5 * ( pow(x -
	x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda,
	2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)
	* lambda, 2)) ) * A * pf * pixel_size * 2 * Pi * lambda * ( cy
	* pow(z - ( z0y + z0) , 2) + s0y)  * lambda) )	/ pow(2 * Pi *
	( cx * pow(z - ( z0x + z0) , 2) + s0x) * lambda * ( cy * pow(z -
	( z0y + z0) , 2) + s0y)  * lambda, 2);
    }

    template <typename Target>
    void derivative( Target target, BestSigma<1> ) {
        target(0,0) = (  - ( ( 2 * Pi * ( cx * pow(z - ( z0x + z0) ,
        2) + s0x)  * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)
        * lambda * A * pf * pixel_size * exp( - 0.5 * ( pow(x - x0,
        2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda, 2)
        + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)
        * lambda, 2)) ) * -0.5 * pow(y - y0, 2) * 2 * lambda * ( cy *
        pow(z - ( z0y + z0) , 2) + s0y)  * lambda)  / pow(( cy * pow(z -
        ( z0y + z0) , 2) + s0y)  * lambda, 4) + exp( - 0.5 * ( pow(x -
        x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda,
        2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)
        * lambda, 2)) ) * A * pf * pixel_size * 2 * Pi * ( cx * pow(z -
        ( z0x + z0) , 2) + s0x)  * lambda * lambda) )  / pow(2 * Pi *
        ( cx * pow(z - ( z0x + z0) , 2) + s0x) * lambda * ( cy * pow(z -
        ( z0y + z0) , 2) + s0y)  * lambda, 2);
    }

    template <typename Target>
    void derivative( Target target, Amplitude ) {
        target(0,0) = ( exp( - 0.5 * ( pow(x - x0, 2) / pow(( cx * pow(z -
        ( z0x + z0) , 2) + s0x)  * lambda, 2) + pow(y - y0, 2) / pow((
        cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 2)) ) * pf *
        pixel_size)  / ( 2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) + s0x)
        * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda) ;
    }

    template <typename Target>
    void derivative( Target target, Prefactor ) {
        target(0,0) = ( exp( - 0.5 * ( pow(x - x0, 2) / pow(( cx * pow(z -
        ( z0x + z0) , 2) + s0x)  * lambda, 2) + pow(y - y0, 2) / pow((
        cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 2)) ) * A *
        pixel_size)  / ( 2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) + s0x)
        * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda) ;
    }

    template <typename Target>
    void derivative( Target target, DeltaSigma<0>  ) {
        target(0,0) = (  - ( ( 2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) +
        s0x)  * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y) * lambda *
        A * pf * pixel_size * exp( - 0.5 * ( pow(x - x0, 2) / pow(( cx *
        pow(z - ( z0x + z0) , 2) + s0x)  * lambda, 2) + pow(y - y0, 2)
        / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 2)) ) *
        -0.5 * pow(x - x0, 2) * 2 * pow(z - ( z0x + z0) , 2) * lambda *
        ( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda)  / pow(( cx *
        pow(z - ( z0x + z0) , 2) + s0x)  * lambda, 4) + exp( - 0.5 *
        ( pow(x - x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  *
        lambda, 2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0) , 2)
        + s0y)  * lambda, 2)) ) * A * pf * pixel_size * 2 * Pi * pow(z -
        ( z0x + z0) , 2) * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)
        * lambda) )  / pow(2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) + s0x)
        * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 2);
    }

    template <typename Target>
    void derivative( Target target, DeltaSigma<1>  ) {
        target(0,0) = (  - ( ( 2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) +
        s0x)  * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y) * lambda *
        A * pf * pixel_size * exp( - 0.5 * ( pow(x - x0, 2) / pow(( cx *
        pow(z - ( z0x + z0) , 2) + s0x)  * lambda, 2) + pow(y - y0, 2)
        / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 2)) ) *
        -0.5 * pow(y - y0, 2) * 2 * pow(z - ( z0y + z0) , 2) * lambda *
        ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda)  / pow(( cy *
        pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 4) + exp( - 0.5 *
        ( pow(x - x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)
        * lambda, 2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0)
        , 2) + s0y)  * lambda, 2)) ) * A * pf * pixel_size * 2 * Pi *
        ( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda * pow(z -
        ( z0y + z0) , 2) * lambda) )  / pow(2 * Pi * ( cx * pow(z -
        ( z0x + z0) , 2) + s0x) * lambda * ( cy * pow(z - ( z0y + z0)
        , 2) + s0y)  * lambda, 2);
    }

    template <typename Target>
    void derivative( Target target, ZOffset<0>  ) {
        target(0,0) = (  - ( ( 2 * Pi * ( cx * pow(z - ( z0x + z0) ,
        2) + s0x)  * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)
        * lambda * A * pf * pixel_size * exp( - 0.5 * ( pow(x - x0,
        2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda, 2)
        + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)
        * lambda, 2)) ) * -0.5 * pow(x - x0, 2) * 2 * cx * -2 * ( z -
        ( z0x + z0) )  * lambda * ( cx * pow(z - ( z0x + z0) , 2) +
        s0x)  * lambda)  / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  *
        lambda, 4) + exp( - 0.5 * ( pow(x - x0, 2) / pow(( cx * pow(z -
        ( z0x + z0) , 2) + s0x) * lambda, 2) + pow(y - y0, 2) / pow((
        cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 2)) ) * A * pf *
        pixel_size * 2 * Pi * cx * -2 * ( z - ( z0x + z0) )  * lambda *
        ( cy * pow(z - ( z0y + z0) , 2) + s0y) * lambda) )  / pow(2 * Pi *
        ( cx * pow(z - ( z0x + z0) , 2) + s0x) * lambda * ( cy * pow(z -
        ( z0y + z0) , 2) + s0y)  * lambda, 2);
    }

    template <typename Target>
    void derivative( Target target, ZOffset<1>  ) {
        target(0,0) = (  - ( ( 2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) +
        s0x)  * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y) * lambda *
        A * pf * pixel_size * exp( - 0.5 * ( pow(x - x0, 2) / pow(( cx *
        pow(z - ( z0x + z0) , 2) + s0x)  * lambda, 2) + pow(y - y0, 2) /
        pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 2)) ) * -0.5
        * pow(y - y0, 2) * 2 * cy * -2 * ( z - ( z0y + z0) )  * lambda *
        ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda)  / pow(( cy *
        pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 4) + exp( - 0.5 *
        ( pow(x - x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)
        * lambda, 2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0)
        , 2) + s0y)  * lambda, 2)) ) * A * pf * pixel_size * 2 * Pi *
        ( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda * cy * -2 *
        ( z - ( z0y + z0) )  * lambda) )  / pow(2 * Pi * ( cx * pow(z -
        ( z0x + z0) , 2) + s0x)  * lambda * ( cy * pow(z - ( z0y + z0)
        , 2) + s0y)  * lambda, 2);
    }

    template <typename Target>
    void derivative( Target target, Wavelength ) {
        target(0,0) = (  - ( 2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) +
        s0x)  * lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y) * lambda *
        0.5 * ( (  - pow(y - y0, 2) * 2 * ( cy * pow(z - ( z0y + z0) ,
        2) + s0y)  * ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda)
        / pow(( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 4) - (
        pow(x - x0, 2) * 2 * ( cx * pow(z - ( z0x + z0) , 2) + s0x)  *
        ( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda) / pow(( cx *
        pow(z - ( z0x + z0) , 2) + s0x)  * lambda, 4))  * exp( - 0.5 *
        ( pow(x - x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)  *
        lambda, 2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0) ,
        2) + s0y)  * lambda, 2)) ) * A * pf * pixel_size + exp( - 0.5 *
        ( pow(x - x0, 2) / pow(( cx * pow(z - ( z0x + z0) , 2) + s0x)
        * lambda, 2) + pow(y - y0, 2) / pow(( cy * pow(z - ( z0y + z0)
        , 2) + s0y)  * lambda, 2)) ) * A * pf * pixel_size * ( 2 * Pi *
        ( cx * pow(z - ( z0x + z0) , 2) + s0x)  * lambda * ( cy * pow(z -
        ( z0y + z0) , 2) + s0y)  + 2 * Pi * ( cx * pow(z - ( z0x + z0) ,
        2) + s0x)  * ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda)
        ) )  / pow(2 * Pi * ( cx * pow(z - ( z0x + z0) , 2) + s0x)  *
        lambda * ( cy * pow(z - ( z0y + z0) , 2) + s0y)  * lambda, 2);
    }

};

template <typename Number, typename P1, typename P2>
class ReferenceEvaluator <No3D, Number, P1, P2>
{
    No3D * const expr;
    static const Number Pi = M_PI;
    Number x, y, x0, y0, s0x, s0y, A, pf, lambda;
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
        lambda = (*expr)( PSF::Wavelength() ).value();
    }
    void value( Eigen::Array<Number,1,1>& result ) 
        { result.fill(0); add_value(result); }
    void add_value( Eigen::Array<Number,1,1>& result ) { 
        result(0,0) += ( exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  *
          lambda, 2) + pow(y - y0, 2) / pow(s0y  * lambda, 2)) ) * A *
          pf * pixel_size)  / ( 2 * Pi * s0x  * lambda * s0y  * lambda) ;
    }

    template <typename Target>
    void derivative( Target t, Amplitude ) {
        t(0,0) = ( exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * lambda, 2)
        + pow(y - y0, 2) / pow(s0y  * lambda, 2)) ) * pf * pixel_size)  /
        ( 2 * Pi * s0x  * lambda * s0y  * lambda) ;
    }

    template <typename Target>
    void derivative( Target t, Prefactor ) {
        t(0,0) = ( exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * lambda, 2)
        + pow(y - y0, 2) / pow(s0y  * lambda, 2)) ) * A * pixel_size)  /
        ( 2 * Pi * s0x  * lambda * s0y  * lambda) ;
    }

    template <typename Target>
    void derivative( Target t, Mean<0>  ) {
        t(0,0) = (  - ( A * pf * pixel_size * exp( - 0.5 * ( pow(x -
        x0, 2) / pow(s0x  * lambda, 2) + pow(y - y0, 2) / pow(s0y  *
        lambda, 2)) ) * ( x0 - x) )  / pow(s0x  * lambda, 2))  / ( 2 *
        Pi * s0x  * lambda * s0y  * lambda) ;
    }

    template <typename Target>
    void derivative( Target t, Mean<1>  ) {
        t(0,0) = (  - ( A * pf * pixel_size * exp( - 0.5 * ( pow(x -
        x0, 2) / pow(s0x  * lambda, 2) + pow(y - y0, 2) / pow(s0y  *
        lambda, 2)) ) * ( y0 - y) )  / pow(s0y  * lambda, 2))  / ( 2 *
        Pi * s0x  * lambda * s0y  * lambda) ;
    }

    template <typename Target, int Dim>
    void derivative( Target target, nonlinfit::Xs<Dim,LengthUnit> ) {
        derivative( target, Mean<Dim>() );
        target(0,0) *= -1;
    }

    template <typename Target>
    void derivative( Target t, BestSigma<0>  ) {
        t(0,0) = (  - ( ( 2 * Pi * s0x  * lambda * s0y  * lambda * A * pf
        * pixel_size * exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * lambda,
        2) + pow(y - y0, 2) / pow(s0y  * lambda, 2)) ) * -0.5 * pow(x -
        x0, 2) * 2 * lambda * s0x  * lambda)  / pow(s0x  * lambda, 4) +
        exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * lambda, 2) + pow(y -
        y0, 2) / pow(s0y  * lambda, 2)) ) * A * pf * pixel_size * 2 *
        Pi * lambda * s0y  * lambda) )  / pow(2 * Pi * s0x  * lambda *
        s0y  * lambda, 2);
    }

    template <typename Target>
    void derivative( Target t, BestSigma<1>  ) {
        t(0,0) = (  - ( ( 2 * Pi * s0x  * lambda * s0y  * lambda * A * pf
        * pixel_size * exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * lambda,
        2) + pow(y - y0, 2) / pow(s0y  * lambda, 2)) ) * -0.5 * pow(y -
        y0, 2) * 2 * lambda * s0y  * lambda)  / pow(s0y  * lambda, 4) +
        exp( - 0.5 * ( pow(x - x0, 2) / pow(s0x  * lambda, 2) + pow(y -
        y0, 2) / pow(s0y  * lambda, 2)) ) * A * pf * pixel_size * 2 *
        Pi * s0x  * lambda * lambda) )  / pow(2 * Pi * s0x  * lambda *
        s0y  * lambda, 2);
    }

    template <typename Target>
    void derivative( Target t, Wavelength ) {
        t(0,0) = (  - ( 2 * Pi * s0x  * lambda * s0y  * lambda * 0.5 *
        ( (  - pow(y - y0, 2) * 2 * s0y  * s0y  * lambda)  / pow(s0y
        * lambda, 4) - ( pow(x - x0, 2) * 2 * s0x  * s0x  * lambda)
        / pow(s0x  * lambda, 4))  * exp( - 0.5 * ( pow(x - x0, 2) /
        pow(s0x  * lambda, 2) + pow(y - y0, 2) / pow(s0y  * lambda,
        2)) ) * A * pf * pixel_size + exp( - 0.5 * ( pow(x - x0, 2) /
        pow(s0x  * lambda, 2) + pow(y - y0, 2) / pow(s0y  * lambda, 2))
        ) * A * pf * pixel_size * ( 2 * Pi * s0x  * lambda * s0y  + 2 *
        Pi * s0x  * s0y  * lambda) ) )  / pow(2 * Pi * s0x  * lambda *
        s0y  * lambda, 2);
    }

};

}
}
}

#endif
