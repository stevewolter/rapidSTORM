#include <simparm/Eigen_decl.h>
#include <simparm/Object.h>
#include <simparm/BoostUnits.h>
#include <simparm/Eigen.h>
#include <simparm/Entry.h>

#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFinderBuilder.h>
#include <dStorm/Direction.h>

using namespace std;
using namespace dStorm::engine;

namespace dStorm {
namespace gauss_smoother {

struct Config {
    typedef Eigen::Matrix< quantity<camera::length>, 2, 1, Eigen::DontAlign > Sigmas;
    simparm::Entry< Sigmas > sigma;
    void attach_ui( simparm::NodeHandle at ) { sigma.attach_ui( at ); }
    Config() 
        : sigma("SmoothingSigma", "Smoothing kernel std.dev.", Sigmas::Constant(1.0 * camera::pixel)) {}
    static std::string get_name() { return "Gaussian"; }
    static std::string get_description() { return "Smooth with gaussian kernel"; }
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
    const double sig_sq = sigma * sigma, norm = 1 / (2 * M_PI * sigma);

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
        fillWithGauss(kernels[i].begin(), size, config.sigma()[i] / camera::pixel, 256);
    }
}

template <typename InputPixel>
void gsm_line(const InputPixel *input, int step, int radius, int size,
          SmoothedPixel *target, std::vector<int>::iterator weights)
{
    for (int c = radius; c < size - radius; c++) {
        int accum = input[c*step] * *weights;
        for (int d = 1; d <= radius; d++) {
            accum += input[(c+d) * step] * *weights;
            accum += input[(c-d) * step] * *weights;
            ++weights;
        }
        target[c*step] = accum;
    }
}

void GaussSmoother::smooth( const engine::Image2D &in )
 
{
    for (int x = 0; x < int(in.height().value()); x++)
        gsm_line( 
            in.ptr(x, 0), in.width().value(),
            kernels[1].size()-1, in.height().value(),
            smoothed.ptr(x, 0), kernels[1].begin() );

    SmoothedPixel copy[smoothed.width().value()];
    for (int y = 0; y < int(smoothed.width_in_pixels()); y++) {
        memcpy(copy, smoothed.ptr(0, y), 
               sizeof(SmoothedPixel) * smoothed.width_in_pixels());
        gsm_line( copy, 1, kernels[0].size()-1, smoothed.width_in_pixels(),
                               smoothed.ptr(0, y), kernels[0].begin() );
    }
}

std::auto_ptr<engine::spot_finder::Factory> make_spot_finder_factory() { 
    return std::auto_ptr<engine::spot_finder::Factory>(
        new engine::spot_finder::Builder<Config,GaussSmoother>()); 
}

}
}
