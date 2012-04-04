#include "dejagnu.h"
#include "Spline.h"
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#include <stdexcept>
#include <fstream>

#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>

#include <dStorm/Localization.h>

namespace dStorm {
namespace threed_info {

struct InterpolationDeleter {
    void operator()( gsl_interp* p ) { gsl_interp_free(p); }
};

SplineFactory::SplineFactory( const std::string& file ) {
    std::ifstream i( file.c_str() );
    while ( i ) {
        double z_in_nm, sigma_x_in_mum, sigma_y_in_mum;
        i >> z_in_nm >> sigma_x_in_mum >> sigma_y_in_mum;
        if ( i )
            add_point( z_in_nm * 1E-9 * si::meter, 
                       sigma_x_in_mum * 1E-6 * si::meter,
                       sigma_y_in_mum * 1E-6 * si::meter );
    }
}

void SplineFactory::add_point( 
    quantity<si::length> z_position, 
    quantity<si::length> sigma_x,
    quantity<si::length> sigma_y )
{
    Spline::Point p;
    p.z = z_position;
    p.sigma[Direction_X] = sigma_x;
    p.sigma[Direction_Y] = sigma_y;
    points.push_back( p );
}

Spline::Spline( const SplineFactory& f )
: N( f.points.size() )
{
    double *zs = new double[N];
    for ( int i = 0; i < N; ++i )
        zs[i] = f.points[i].z.value();
    this->zs.reset( zs );

    for ( Direction dim = Direction_First; dim < Direction_2D; ++dim ) {
        double *v = new double[N];
        for ( int i = 0; i < N; ++i )
            v[i] = f.points[i].sigma[dim].value();
        sigmas[dim].reset( v );

        gsl_interp* spline = gsl_interp_alloc(gsl_interp_cspline, N );
        gsl_interp_init( spline, this->zs.get(), sigmas[dim].get(), N );
        splines[dim].reset( spline, InterpolationDeleter() );
    }

    ZPosition maybe_equifocal_plane = look_up_sigma_diff( 0 * si::meter, 1 * si::meter );
    if ( maybe_equifocal_plane )
        equifocal_plane_ = *maybe_equifocal_plane;
    else
        throw std::runtime_error("Z spline has no point where widths are equal");
}

Spline::Sigma Spline::get_sigma_diff( quantity<si::length> z ) const { 
    Spline::Sigma
        x = get_sigma( Direction_X, z ),
        y = get_sigma( Direction_Y, z );
    if ( x && y ) 
        return *x - *y; 
    else 
        return Spline::Sigma();
}

Spline::Sigma Spline::get_sigma( Direction dir, quantity<si::length> z ) const {
    double result;
    int error = gsl_interp_eval_e( splines[dir].get(), zs.get(), sigmas[dir].get(), z.value(), NULL, &result );
    if ( error == GSL_SUCCESS )
        return quantity<si::length>::from_value( result );
    else if ( error == GSL_EDOM )
        return Spline::Sigma();
    else
        throw std::logic_error("Unexpected error in Z depth spline range");
}

Spline::SigmaDerivative
Spline::get_sigma_deriv( Direction dir, quantity<si::length> z ) const {
    double result;
    int error = gsl_interp_eval_deriv_e( splines[dir].get(), zs.get(), sigmas[dir].get(), z.value(), NULL, &result );
    if ( error == GSL_SUCCESS )
        return result;
    else if ( error == GSL_EDOM )
        return Spline::SigmaDerivative();
    else
        throw std::logic_error("Unexpected error in Z depth spline range");
}

Spline::ZPosition Spline::look_up_sigma_diff( 
    const Localization& l, quantity<si::length> precision
) const
{
    return look_up_sigma_diff( 
        sqrt( l.fit_covariance_matrix()(0,0) ),
        sqrt( l.fit_covariance_matrix()(1,1) ),
        precision );
}

Spline::ZPosition Spline::look_up_sigma_diff( 
    quantity<si::length> sigma_x, quantity<si::length> sigma_y ,
    quantity<si::length> precision
) const
{
    return look_up_sigma_diff( sigma_x - sigma_y, precision );
}

Spline::ZPosition Spline::look_up_sigma_diff( 
    quantity<si::length> searched,
    quantity<si::length> precision
) const
{
    quantity<si::length> lower_bound = zs[0] * si::meter, upper_bound = zs[N-1] * si::meter;

    /* Switch bounds if the gradient is negative, so we can assume in 
     * the rest of the search that lower_bound is at the Y-larger-X end
     * (not necessarily the low-Z end). */
    if ( *get_sigma_diff( lower_bound ) > *get_sigma_diff( upper_bound ) )
        std::swap( lower_bound, upper_bound );
    assert( *get_sigma_diff( lower_bound ) < *get_sigma_diff( upper_bound ) );

    if ( *get_sigma_diff(lower_bound) > searched )
        return Spline::ZPosition();
    else if ( *get_sigma_diff(upper_bound) < searched)
        return Spline::ZPosition();
    else {
        while ( abs( upper_bound - lower_bound ) > precision ) {
            quantity<si::length> test_x = (lower_bound + upper_bound) / 2.0;
            quantity<si::length> test_y = *get_sigma_diff( test_x );
            if ( test_y > searched )
                upper_bound = test_x;
            else
                lower_bound = test_x;
        }

        return (upper_bound + lower_bound) / 2.0;
    }
}

std::pair< Spline::ZPosition, Spline::ZPosition > Spline::get_range() const {
    std::pair< ZPosition, ZPosition > rv;
    rv.first = std::min( zs[0], zs[N-1] ) * si::meter;
    rv.second = std::max( zs[0], zs[N-1] ) * si::meter;
    return rv;
}

static double spline_test_data[][3] = {
    { 72.0, 0.551030474849, 0.295956171647}, 
    { 152.0, 0.556805487709, 0.299537279667}, 
    { 232.0, 0.550644094748, 0.300626731964}, 
    { 312.0, 0.548091958375, 0.307305393008}, 
    { 392.0, 0.549973430715, 0.31146738838}, 
    { 472.0, 0.547133231488, 0.305487066472}, 
    { 552.0, 0.537197635972, 0.295457250469}, 
    { 632.0, 0.512142340106, 0.278675831978}, 
    { 712.0, 0.488657906409, 0.261683342022}, 
    { 792.0, 0.481411209001, 0.25150256443}, 
    { 872.0, 0.478922901766, 0.243956058207}, 
    { 952.0, 0.465998362506, 0.233581833233}, 
    { 1032.0, 0.452354553298, 0.224737661247}, 
    { 1112.0, 0.446078964461, 0.221125496197}, 
    { 1192.0, 0.439620503455, 0.218066407607}, 
    { 1272.0, 0.429141702973, 0.212888607881}, 
    { 1352.0, 0.415245425713, 0.207118976285}, 
    { 1432.0, 0.399451802334, 0.201726304774}, 
    { 1512.0, 0.384377031508, 0.195631958859}, 
    { 1592.0, 0.37272404242, 0.189331044449}, 
    { 1672.0, 0.360866254633, 0.182821846317}, 
    { 1752.0, 0.348277158662, 0.176823239551}, 
    { 1832.0, 0.337716709385, 0.17201927589}, 
    { 1912.0, 0.327407697418, 0.167715137606}, 
    { 1992.0, 0.316653630973, 0.162842980648}, 
    { 2072.0, 0.306575043327, 0.158075885823}, 
    { 2152.0, 0.297262726623, 0.153821939757}, 
    { 2232.0, 0.287767083237, 0.149778597498}, 
    { 2312.0, 0.278399596663, 0.146051214967}, 
    { 2392.0, 0.268687670408, 0.143484021057}, 
    { 2472.0, 0.257884214787, 0.141897826642}, 
    { 2552.0, 0.246991478979, 0.140485339824}, 
    { 2632.0, 0.236929841418, 0.139187497061}, 
    { 2712.0, 0.227127648172, 0.138467642898}, 
    { 2792.0, 0.217054390829, 0.138490194353}, 
    { 2872.0, 0.207273995865, 0.139167911954}, 
    { 2952.0, 0.198325546494, 0.140439778373}, 
    { 3032.0, 0.190309467391, 0.14268739887}, 
    { 3112.0, 0.182637283544, 0.146251802613}, 
    { 3192.0, 0.175392362642, 0.151087865627}, 
    { 3272.0, 0.169131214513, 0.157597340738}, 
    { 3352.0, 0.164042695461, 0.167383754465}, 
    { 3432.0, 0.159988052214, 0.183470836573}, 
    { 3512.0, 0.157451780193, 0.208298900021}, 
    { 3592.0, 0.155944468229, 0.239354792115}, 
    { 3672.0, 0.15496094371, 0.272136369304}, 
    { 3752.0, 0.154633146586, 0.307016899546}, 
    { 3832.0, 0.154903758151, 0.346859408797}, 
    { 3912.0, 0.1558978651, 0.389819668851}, 
    { 3992.0, 0.156646332553, 0.413667317271}, 
};

SplineFactory SplineFactory::Mock() {
    SplineFactory rv;
    for (size_t i = 0; i < sizeof(spline_test_data) / sizeof(spline_test_data[0]); ++i)
        rv.add_point( spline_test_data[i][0] * 1E-9 * si::meter,
                     spline_test_data[i][1] * 1E-6 * si::meter,
                     spline_test_data[i][2] * 1E-6 * si::meter );
    return rv;
}

void unit_tests( TestState& state ) {
    SplineFactory f = SplineFactory::Mock();

    Spline s( f );
    Spline::Sigma s1 = s.get_sigma(Direction_X, 1.6E-6 * si::meter);
    state( s1 && abs( *s1 - 0.37E-6 * si::meter ) < 10E-9 * si::meter,
           "Spline gives good X values" );
    Spline::Sigma s2 = s.get_sigma(Direction_Y, 2.1E-6 * si::meter);
    state( s1 && abs( *s2 - 0.153E-6 * si::meter ) < 10E-9 * si::meter,
           "Spline gives good Y values" );
    Spline::Sigma s3 = s.get_sigma(Direction_X, 5.0E-6 * si::meter);
    state( ! s3, "Spline reacts gracefully to domain error" );

    const int ti = 25;
    Spline::ZPosition z1 = s.look_up_sigma_diff( 
        spline_test_data[ti][1] * 1E-6 * si::meter + 3.141E-10 * si::meter,
        spline_test_data[ti][2] * 1E-6 * si::meter - 2.718E-10 * si::meter,
        1E-9 * si::meter );
    state( z1 && *z1 < spline_test_data[ti][0] * 1E-9 * si::meter 
              && *z1 > spline_test_data[ti-1][0] * 1E-9 * si::meter,
           "Z determination through sigmadiff works" );
}

}
}
