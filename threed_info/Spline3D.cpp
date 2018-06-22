#include "dejagnu.h"
#include "threed_info/Spline3D.h"
#include "threed_info/Config.h"
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#include <stdexcept>
#include <fstream>
#include <simparm/Object.h>
#include <simparm/FileEntry.h>

#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>

#include "Localization.h"
#include "threed_info/look_up_sigma_diff.h"
#include <Eigen/Dense>

namespace dStorm {
namespace threed_info {

struct InterpolationDeleter {
    void operator()( gsl_interp* p ) { gsl_interp_free(p); }
};

SplineFactory::SplineFactory( const std::string& file, Direction dir ) 
{
    std::ifstream i( file.c_str() );
    while ( i ) {
        float z_in_nm, sigma_in_mum[2];
        i >> z_in_nm >> sigma_in_mum[0] >> sigma_in_mum[1];
        if ( i )
            points.push_back( Spline3D::Point{z_in_nm * 1E-3, sigma_in_mum[dir]} );
    }
}

Spline3D::Spline3D( const SplineFactory& f )
: N( f.points.size() ),
  points( f.points ),
  h( f.points[1].z - f.points[0].z )
{
    if ( N <= 3 ) throw std::runtime_error("Need at least 4 points for Z-sigma interpolation");
    /* Construction of spline linear equation system according to
        * McKinley and Levine, Cubic Spline Interpolation.
        * Natural runout conditions to match GSL implementation. */
    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(N-2,N-2);
    Eigen::VectorXd B( N - 2 );
    for (int j = 0; j < N-2; ++j) {
        B[j] = points[j].sigma - 2 * points[j+1].sigma + points[j+2].sigma;

        A(j,j) = 4;
        if ( j > 0 ) A(j,j-1) = 1;
        if ( j < N - 3 ) A(j,j+1) = 1;
    }
    B *= 6 / (h*h);
    Eigen::VectorXd M = Eigen::VectorXd::Zero(N);
    M.block( 1, 0, N-2, 1 ) = A.colPivHouseholderQr().solve(B);
    M[0] = 0;
    M[N-1] = 0;
    coeffs = Eigen::MatrixXd::Zero( N-1, 4 );
    for (int i = 0; i < N-1; ++i) {
        coeffs(i,3) = points[i].sigma;
        coeffs(i,2) = (points[i+1].sigma - points[i].sigma) / h
            - ( M[i+1] + 2 * M[i] ) * h / 6;
        coeffs(i,1) = M[i] / 2;
        coeffs(i,0) = (M[i+1] - M[i]) / (6 * h);
    }
}

ZRange Spline3D::z_range_() const { 
    ZRange rv;
    rv.insert( boost::icl::continuous_interval<ZPosition>( 
        points[0].z, points[N-1].z ) ); 
    return rv;
}

Sigma Spline3D::get_sigma_( ZPosition z ) const {
    int i = std::max( 0, std::min( int( floor( (z - points[0].z) / h ) ), N-2 ) );
    double rv = 0, dx = z - points[i].z;
    for (int term = 0; term < 4; ++term)
        rv = rv * dx + coeffs(i,term);
    return rv;
}

SigmaDerivative
Spline3D::get_sigma_deriv_( ZPosition z ) const {
    int i = std::max( 0, std::min( int( floor( (z - points[0].z) / h ) ), N-2 ) );
    double rv = 0, dx = z - points[i].z;
    for (int term = 0; term < 3; ++term)
        rv = rv * dx + (3-term) * coeffs(i,term);
    return rv;
}

class Spline3DConfig : public Config {
    simparm::FileEntry z_calibration_file;

    boost::shared_ptr<DepthInfo> make_traits( Direction dir ) const {
        if ( z_calibration_file )
            return boost::shared_ptr<DepthInfo>(new Spline3D( SplineFactory( z_calibration_file(), dir) ));
        else
            return boost::shared_ptr<DepthInfo>();
    }
    void read_traits( const DepthInfo&, const DepthInfo& ) 
        { z_calibration_file = ""; }
    void set_context() {}
    void attach_ui( simparm::NodeHandle to ) { 
        simparm::NodeHandle r = attach_parent(to); 
        z_calibration_file.attach_ui( r );
    }
  public:
    Spline3DConfig() 
        : Config("Spline3D"),
          z_calibration_file("ZCalibration", "") {}
    Spline3DConfig* clone() const { return new Spline3DConfig(*this); }
};

std::auto_ptr< Config > make_spline_3d_config()
    { return std::auto_ptr< Config >( new Spline3DConfig() ); }


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

SplineFactory SplineFactory::Mock( Direction d ) {
    SplineFactory rv;
    for (size_t i = 0; i < sizeof(spline_test_data) / sizeof(spline_test_data[0]); ++i)
        rv.points.push_back( Spline3D::Point{ 
            spline_test_data[i][0] * 1E-3,
            spline_test_data[i][1+d-Direction_X] });
    return rv;
}

void unit_tests( TestState& state ) {
    Spline3D sx( SplineFactory::Mock(Direction_X) );
    Spline3D sy( SplineFactory::Mock(Direction_Y) );

    Sigma s1 = sx.get_sigma(1.6);
    state( abs( s1 - 0.37 ) < 10E-3,
           "Spline gives good X values" );
    Sigma s2 = sy.get_sigma(2.1);
    state( abs( s2 - 0.153 ) < 10E-3,
           "Spline gives good Y values" );
    Sigma s4 = sx.get_sigma(3.992);
    state( abs( s4 - 0.15664 ) < 1E-3,
           "Spline catches terminal point" );
    Sigma s3 = sx.get_sigma(4.0);
    state( s3 > s4 && (s3 - s4) < 0.3,
           "Spline can extrapolate X values" );

    const int ti = 25;
    SigmaDiffLookup lu( sx, sy, 1e-3 );
    ZPosition z1 = lu( 
        Sigma(spline_test_data[ti][1] + 3.141E-4),
        Sigma(spline_test_data[ti][2] - 2.718E-4) );
    state( z1 < spline_test_data[ti][0] * 1E-3 &&
           z1 > spline_test_data[ti-1][0] * 1E-3,
           "Z determination through sigmadiff works" );

    ZPosition dx = 1E-4;
    Sigma slow = sx.get_sigma(2.3 - dx);
    Sigma shigh = sx.get_sigma(2.3 + dx);
    double slope = sx.get_sigma_deriv( 2.3 );
    state( std::abs( (shigh - slow) / (2.0f*dx) - slope ) < std::abs(slope) * 0.1,
           "Sigma derivative is correct on interpolation" );
}

}
}
