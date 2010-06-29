#include "Exponential3D_Accessor.h"
#include "Exponential3D_Derivatives.h"

using namespace fitpp::Exponential3D;

typedef Model< 1 > Space;
typedef Deriver< 1, 9, 11 > ToTest;
typedef Eigen::Matrix<double,11,9> Data;

void compute_residues_naively(
    const Space::Accessor& parameters,
    const ToTest::Data& data,
    ToTest::Data& residues,
    Space::Vector& gradient,
    Space::Matrix& hessian
) {
    residues.fill(0);
    gradient.fill(0);
    hessian.fill(0);

    double x0 = parameters.getMeanX<0>().value(),
           y0 = parameters.getMeanY<0>().value(),
           z = parameters.getMeanZ<0>().value(),
           zx = parameters.getZAtBestSigmaX<0>().value(),
           zy = parameters.getZAtBestSigmaY<0>().value(),
           cx = pow(parameters.getDeltaSigmaX<0>().value(),2),
           cy = pow(parameters.getDeltaSigmaY<0>().value(),2),
           s0x = sqrt(parameters.getBestVarianceX<0>().value()),
           s0y = sqrt(parameters.getBestVarianceY<0>().value()),
           A = parameters.getAmplitude<0>().value(),
           B = parameters.getShift().value(),
           Pi = M_PI;

    double derivs[5];
    for (int row = 0; row < data.rows(); ++row)
      for (int col = 0; col < data.cols(); ++col)
      {
        double x = col, y = row;
        double value = ( exp( - 0.5 * ( pow(x - x0, 2) / ( cx * pow(z - zx, 2) + 
            pow(s0x, 2))  + pow(y - y0, 2) / ( cy * pow(z - zy, 2) + 
            pow(s0y, 2)) ) ) * A) / ( 2 * Pi * sqrt(( cx * pow(z - zx, 2) + 
            pow(s0x, 2))  * ( cy * pow(z - zy, 2) + pow(s0y, 2)) ))  + B;
        derivs[Shift] = 1;
        derivs[Amplitude] = exp( - 0.5 * ( pow(x - x0, 2) / 
            ( cx * pow(z - zx, 2) + pow(s0x, 2))  + pow(y - y0, 2) / ( cy * 
                pow(z - zy, 2) + pow(s0y, 2)) ) ) / ( 2 * Pi * sqrt(( cx * 
                pow(z - zx, 2) + pow(s0x, 2))  * ( cy * pow(z - zy, 2) + 
                pow(s0y, 2)) )) ;
    derivs[MeanX] = (  - ( A * exp( - 0.5 * ( pow(x - x0, 2) / ( cx * pow(z - zx, 2) + pow(s0x, 2))  + pow(y - y0, 2) / ( cy * pow(z - zy, 2) + pow(s0y, 2)) ) ) * ( x - x0) )  / ( cx * pow(z - zx, 2) + pow(s0x, 2)) )  / ( 2 * Pi * sqrt(( cx * pow(z - zx, 2) + pow(s0x, 2))  * ( cy * pow(z - zy, 2) + pow(s0y, 2)) )) ;
    derivs[MeanY] = (  - ( A * exp( - 0.5 * ( pow(x - x0, 2) / ( cx * pow(z - zx, 2) + pow(s0x, 2))  + pow(y - y0, 2) / ( cy * pow(z - zy, 2) + pow(s0y, 2)) ) ) * ( y - y0) )  / ( cy * pow(z - zy, 2) + pow(s0y, 2)) )  / ( 2 * Pi * sqrt(( cx * pow(z - zx, 2) + pow(s0x, 2))  * ( cy * pow(z - zy, 2) + pow(s0y, 2)) )) ;
    derivs[MeanZ] = (  - ( 2 * Pi * sqrt(( cx * pow(z - zx, 2) + pow(s0x, 2))  * ( cy * pow(z - zy, 2) + pow(s0y, 2)) ) * 0.5 * ( (  - pow(y - y0, 2) * cy * 2 * ( z - zy) )  / pow(cy * pow(z - zy, 2) + pow(s0y, 2), 2) - ( pow(x - x0, 2) * cx * 2 * ( z - zx) )  / pow(cx * pow(z - zx, 2) + pow(s0x, 2), 2))  * exp( - 0.5 * ( pow(x - x0, 2) / ( cx * pow(z - zx, 2) + pow(s0x, 2))  + pow(y - y0, 2) / ( cy * pow(z - zy, 2) + pow(s0y, 2)) ) ) * A + ( exp( - 0.5 * ( pow(x - x0, 2) / ( cx * pow(z - zx, 2) + pow(s0x, 2))  + pow(y - y0, 2) / ( cy * pow(z - zy, 2) + pow(s0y, 2)) ) ) * A * 2 * Pi * ( ( cx * pow(z - zx, 2) + pow(s0x, 2))  * cy * 2 * ( z - zy)  + cx * 2 * ( z - zx)  * ( cy * pow(z - zy, 2) + pow(s0y, 2)) ) )  / ( 2 * sqrt(( cx * pow(z - zx, 2) + pow(s0x, 2))  * ( cy * pow(z - zy, 2) + pow(s0y, 2)) )) ) )  / pow(2 * Pi * sqrt(( cx * pow(z - zx, 2) + pow(s0x, 2))  * ( cy * pow(z - zy, 2) + pow(s0y, 2)) ), 2);
        
        residues(row,col) = data(row,col) - value;
        for (int i = 0; i < 5; ++i) {
            gradient[i] += residues(row,col) * derivs[i];
            for (int j = 0; j < 5; ++j) {
                hessian(i,j) += derivs[i] * derivs[j];
            }
        }
      }
}


int main() {
    ToTest::Space::Variables vars;
    ToTest::Space::Accessor parameters(&vars);
    Data data;

    parameters.setMeanX<0>( 5 * cs_units::camera::pixel );
    parameters.setMeanY<0>( 2 * cs_units::camera::pixel );
    parameters.setMeanZ<0>( 35 * boost::units::si::nanometre );
    parameters.setAmplitude<0>( 2000 * cs_units::camera::ad_count );
    parameters.setDeltaSigmaX<0>( 0.02 * cs_units::camera::pixels_per_meter );
    parameters.setDeltaSigmaY<0>( 0.03 * cs_units::camera::pixels_per_meter );
    parameters.setBestVarianceX<0>( 1.3*1.3 * cs_units::camera::pixel * cs_units::camera::pixel );
    parameters.setBestVarianceY<0>( 1.2*1.2 * cs_units::camera::pixel * cs_units::camera::pixel );
    parameters.setZAtBestSigmaX<0>( -100 * boost::units::si::nanometre );
    parameters.setZAtBestSigmaY<0>( +200 * boost::units::si::nanometre );
    parameters.setShift( 15 * cs_units::camera::ad_count );
    srand(50);
    for (int r = 0; r < data.rows(); ++r) 
      for (int c = 0; c < data.cols(); ++c) 
        data(r,c) = rand() * 1E-5;

    Data residues_naive, residues_optimized;
    Space::Vector gradient_naive, gradient_optimized;
    Space::Matrix hessian_naive, hessian_optimized;
    
    compute_residues_naively( parameters, data, residues_naive, gradient_naive, hessian_naive );

    Deriver<1, 9, 11> test;
    test.prepare( vars, parameters.getConstants(), 0, 0 );
    test.compute( data, residues_optimized, gradient_optimized, hessian_optimized );

#define CHECK(x) \
    if ( ( (x ## _naive - x ## _optimized).cwise().abs().cwise() > 1E-8 ).any() ) { \
        std::cerr << x ## _naive << "\n\n" << x ## _optimized << "\n\n" << (x ## _naive - x ## _optimized) << std::endl; \
        std::cerr << std::endl; \
        return 1; \
    }
    CHECK(residues);
    CHECK(gradient);
    CHECK(hessian);

    return 0;
}
