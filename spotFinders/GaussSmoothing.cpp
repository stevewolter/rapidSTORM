#include <boost/math/constants/constants.hpp>

#include "simparm/Eigen_decl.h"
#include "simparm/Object.h"
#include "simparm/BoostUnits.h"
#include "simparm/Eigen.h"
#include "simparm/Entry.h"

#include "engine/SpotFinder.h"
#include "engine/SpotFinderBuilder.h"
#include "Direction.h"
#include "simparm/GUILabelTable.h"
#include "helpers/make_unique.hpp"

using namespace std;
using namespace dStorm::engine;

namespace dStorm {
namespace gauss_smoother {

struct Config {
    typedef Eigen::Matrix< quantity<camera::length>, 2, 1, Eigen::DontAlign > Sigmas;
    simparm::Entry< Sigmas > sigma;
    void attach_ui( simparm::NodeHandle at ) { sigma.attach_ui( at ); }
    Config() 
        : sigma("SmoothingSigma", Sigmas::Constant(1.0 * camera::pixel)) {}
    static std::string get_name() { return "Gaussian"; }
    static std::string get_description() { return simparm::GUILabelTable::get_singleton().get_description( get_name() ); }
};

class GaussSmoother : public engine::spot_finder::Base {
public:
    GaussSmoother (const Config&, const engine::spot_finder::Job&);
    GaussSmoother* clone() const { return new GaussSmoother(*this); }

    void smooth( const engine::Image2D &in );

private:
    std::vector<int> kernels[Direction_2D];
};

static void fillWithGauss(std::vector<int>::iterator values, int n, double sigma, int A) {
    const double sig_sq = sigma * sigma, norm = 1 / (2 * boost::math::constants::pi<double>() * sigma);

    for (int i = 0; i < n; i++)
        *values++ = (int)round(A * norm * exp( -0.5 * i * i / sig_sq ) );
}

GaussSmoother::GaussSmoother (
    const Config& config, const engine::spot_finder::Job &job)
: Base(job)
{
    for (int i = 0; i < 2; ++i) {
        double sigma = config.sigma()[i] / camera::pixel;
        const int size = int(ceil(2 * sigma))+1;
        kernels[i].resize( size, 0 );
        fillWithGauss(kernels[i].begin(), size, config.sigma()[i] / camera::pixel, 256);
    }
}

template <typename InputPixel>
static void gsm_line(const InputPixel *input, int step, int size,
          SmoothedPixel *target, const std::vector<int>& weights)
{
    const int radius = weights.size();
    for (int c = 0; c < size; ++c) {
        int accum = 0;
        for (int d = 0; d < radius; d++) {
            accum += input[std::min(c+d, size-1) * step] * weights[d];
            if ( d > 0 )
                accum += input[std::max(c-d, 0) * step] * weights[d];
        }
        target[c*step] = accum;
    }
}

void GaussSmoother::smooth( const engine::Image2D &in )
 
{
    for (int x = 0; x < int(in.height().value()); x++)
        gsm_line( 
            in.ptr(x, 0), in.width_in_pixels(),
            in.height_in_pixels(), smoothed.ptr(x, 0), kernels[1] );

    SmoothedPixel copy[smoothed.width_in_pixels()];
    for (int y = 0; y < int(smoothed.height_in_pixels()); y++) {
        memcpy(copy, smoothed.ptr(0, y), 
               sizeof(SmoothedPixel) * smoothed.width_in_pixels());
        gsm_line( copy, 1, smoothed.width_in_pixels(),
                           smoothed.ptr(0, y), kernels[0] );
    }
}

std::unique_ptr<engine::spot_finder::Factory> make_spot_finder_factory() { 
    return make_unique<engine::spot_finder::Builder<Config,GaussSmoother>>(); 
}

}
}
