#include "dejagnu.h"
#include "Spline.h"
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#include <stdexcept>

#include <boost/units/cmath.hpp>

namespace dStorm {
namespace threed_info {

struct InterpolationDeleter {
    void operator()( gsl_interp* p ) { gsl_interp_free(p); }
};

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

Spline::Spline( const SplineFactory& f ) {
    const int n = f.points.size();

    double *zs = new double[n];
    for ( int i = 0; i < n; ++i )
        zs[i] = f.points[i].z.value();
    this->zs.reset( zs );

    for ( Direction dim = Direction_First; dim < Direction_2D; ++dim ) {
        double *v = new double[n];
        for ( int i = 0; i < n; ++i )
            v[i] = f.points[i].sigma[dim].value();
        sigmas[dim].reset( v );

        gsl_interp* spline = gsl_interp_alloc(gsl_interp_cspline, n );
        gsl_interp_init( spline, this->zs.get(), sigmas[dim].get(), n );
        splines[dim].reset( spline, InterpolationDeleter() );
    }
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
{ 1832.0, 0.33697241732, 0.171794068617}, 
{ 1912.0, 0.318928101494, 0.16406456734}, 
{ 1984.0, 0.295888860793, 0.152139013625}, 
{ 2032.0, 0.278801134085, 0.142476079373}, 
{ 2072.0, 0.278068924608, 0.143803271828}, 
{ 2136.0, 0.282387729365, 0.154730155299}, 
{ 2208.0, 0.283036765124, 0.159494992866}, 
{ 2288.0, 0.277954873374, 0.152865261629}, 
{ 2368.0, 0.272316402582, 0.146058253157}, 
{ 2448.0, 0.266399782389, 0.14945187929}, 
{ 2520.0, 0.261897754708, 0.154970399332}, 
{ 2600.0, 0.263045079303, 0.162798730511}, 
{ 2672.0, 0.255136951598, 0.161389768318}, 
{ 2752.0, 0.23452925288, 0.151429934748}, 
{ 2832.0, 0.213590525812, 0.150395730922}, 
{ 2912.0, 0.205007175546, 0.158933136867}, 
{ 2984.0, 0.200480688286, 0.159624711596}, 
{ 3056.0, 0.20103717932, 0.162003179075}, 
{ 3128.0, 0.201523826297, 0.168845128503}, 
{ 3208.0, 0.211917010063, 0.182271134606}, 
{ 3272.0, 0.233354500667, 0.20318934981}, 
{ 3336.0, 0.234488615432, 0.213218752284}, 
{ 3408.0, 0.209995111089, 0.218054122065}, 
{ 3488.0, 0.203983717526, 0.246904350234}, 
{ 3536.0, 0.228498923648, 0.278229077169}, 
{ 3592.0, 0.243658780106, 0.292885427933}, 
{ 3648.0, 0.226239410026, 0.321396761868}, 
{ 3712.0, 0.208922609823, 0.369598839322}, 
{ 3776.0, 0.232276580138, 0.384408412673}, 
{ 3840.0, 0.259603263645, 0.372764832237}, 
{ 3896.0, 0.232332206956, 0.383023276823}, 
{ 3976.0, 0.182174896815, 0.411367975485}, 
};

void unit_tests( TestState& state ) {
    SplineFactory f;
    for (size_t i = 0; i < sizeof(spline_test_data) / sizeof(spline_test_data[0]); ++i)
        f.add_point( spline_test_data[i][0] * 1E-9 * si::meter,
                     spline_test_data[i][1] * 1E-6 * si::meter,
                     spline_test_data[i][2] * 1E-6 * si::meter );

    Spline s( f );
    Spline::Sigma s1 = s.get_sigma(Direction_X, 1.6E-6 * si::meter);
    state( s1 && abs( *s1 - 0.37E-6 * si::meter ) < 10E-9 * si::meter,
           "Spline gives good X values" );
    Spline::Sigma s2 = s.get_sigma(Direction_Y, 2.1E-6 * si::meter);
    state( s1 && abs( *s2 - 0.145E-6 * si::meter ) < 10E-9 * si::meter,
           "Spline gives good Y values" );
    Spline::Sigma s3 = s.get_sigma(Direction_X, 5.0E-6 * si::meter);
    state( ! s3, "Spline reacts gracefully to domain error" );
}

}
}
