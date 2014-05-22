#include "simparm/Object.h"
#include "engine/SpotFinder.h"
#include "engine/SpotFinderBuilder.h"
#include "simparm/FileEntry.h"
#include "output/Output.h"
#include "output/OutputBuilder.h"
#include "output/FileOutputBuilder.h"
#include "engine/Image.h"
#include "engine/Input.h"

#include "image/dilation.h"
#include "image/morphological_reconstruction.hpp"
#include "spotFinders/Fillhole.h"
#include "image/iterator.h"

#include "image/convert.h"
#include "image/constructors.h"
#include "simparm/GUILabelTable.h"

namespace dStorm {
namespace fillhole_smoother {

class Config 
{
    public:
    /** Mask sizes */
    simparm::Entry<unsigned long> spots;

    Config() 
    : spots("SpotReconstructionMaskSize", 3) {}
    void attach_ui( simparm::NodeHandle at ) {
        spots.attach_ui(at);
    }
    static std::string get_name() { return "Reconstruction"; }
    static std::string get_description() { return simparm::GUILabelTable::get_singleton().get_description( get_name() ); }
};

class FillholeSmoother : public engine::spot_finder::Base {
private:
    engine::SmoothedImage buffer[3];
    int rms1;

public:
    FillholeSmoother (const Config& myconf,
                        const dStorm::engine::spot_finder::Job &conf);
    ~FillholeSmoother() {}
    FillholeSmoother* clone() const { return new FillholeSmoother(*this); }

    void smooth( const dStorm::engine::Image2D &in );
};

/** Produce an image that is unchanged throughout but for the border of
 *  width \c border, which is from \c source. */
template <typename T1, typename T2> 
    void fill_border(dStorm::Image<T1,2>& res, int border, const dStorm::Image<T2,2>& src)
{
    for (int i = 0; i < border; i++) {
        for (int x = 0; x < res.width_in_pixels(); x++) {
            res(i, x) = src(i, x);
            res(res.height_in_pixels()-1-i,x) =
                src(src.height_in_pixels()-1-i, x);
        }
        for (int y = 0; y < res.height_in_pixels(); y++) {
            res(y, i) = src(y, i);
            res(y, res.width_in_pixels()-1-i) 
                = src(y, res.width_in_pixels()-1-i);
        }
    }
}

template <typename Pixel>
struct Invert {
    const Pixel max;
    Invert() : max( std::numeric_limits<Pixel>::max() ) {}
    Pixel operator()(Pixel p) const { return max - p; }
};

void
FillholeSmoother::smooth( const dStorm::engine::Image2D &image ) {
        engine::SmoothedImage &inv_image = buffer[0],
                      &inv_fillhole_mask = buffer[1],
                      &recon = buffer[2];
        /* Remember that reconstruction by erosion is equivalent
         * to the invert of reconstruction by dilation using the
         * inverts of the mask and marker image. */
        std::transform( image.begin(), image.end(), inv_image.begin(), 
                        Invert<dStorm::engine::SmoothedPixel>() );

        inv_fillhole_mask.fill(0);
        fill_border(inv_fillhole_mask, 2, inv_image);

        /* This is effectively the fillhole transformation on image with
         * the inverted result stored in recon. */
        image::reconstruction_by_dilation
            (inv_fillhole_mask, inv_image, recon);
            
        std::transform( recon.begin(), recon.end(), recon.begin(), 
                        Invert<dStorm::engine::SmoothedPixel>() );
        /* The black border introduced by the dilation (as boundary) would be white now.
         * We need to reconstruct the border. Since we performed a fillhole, by definition
         * the border is unchanged, so re-copy it. */
        fill_border(recon, 1, image);

        /* Code for removal of uneven background. Not necessary as of now. */
        //rectangular_erosion( recon, bg, rms2/2, rms2/2, 0, 0 );
        //rectangular_dilation( bg, bg, rms2/2, rms2/2, 0, 0 );

        rectangular_erosion( recon, smoothed, rms1/2, rms1/2, 0, 0 );
}

FillholeSmoother::FillholeSmoother(
    const Config& myConf, const dStorm::engine::spot_finder::Job& job)
: Base(job),
    rms1(myConf.spots())
{
    for (int i = 0; i < 3; i++)
        buffer[i] = engine::SmoothedImage( job.size().head<2>() );
}

std::auto_ptr< engine::spot_finder::Factory >
    make_fillhole_smoother()
{
    return std::auto_ptr< engine::spot_finder::Factory >(
        new engine::spot_finder::Builder<Config,FillholeSmoother>() );
}

}
}
