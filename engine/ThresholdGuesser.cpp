#include "debug.h"
#include "ThresholdGuesser.h"
#include <limits>
#include <dStorm/image/iterator.h>

namespace dStorm {
namespace engine {

static void add_to_histogram( const Image& im, int h[], const int b ) 
{
    for ( Image::const_iterator i = im.begin(), e = im.end(); i != e; ++i ) 
    {
        h[ *i >> b ] += 1;
    }
}

ThresholdGuesser::ThresholdGuesser( Input& input )
: input(input),
  confidence_limit(8),
  binning(3)
{
}

boost::units::quantity<cs_units::camera::intensity,float>
ThresholdGuesser::compute_threshold()
{
    int hist_size = (int(std::numeric_limits<StormPixel>::max())+1) >> binning;
    int histogram[hist_size];
    for (int i = 0; i < hist_size; i++) histogram[i] = 0;

    double mean = -1, variance = -1;
    Input::iterator i, e = input.end();
    for ( i = input.begin(); i != e; ++i ) {
        DEBUG("Got image of size " << i->width_in_pixels() << " " << i->height_in_pixels());
        DEBUG("Adding image to histogram");
        add_to_histogram( *i, histogram, binning );
        
        int highest_bin = 0;
        for (int j = 1; j < hist_size; ++j)  {
            //DEBUG("Image " << i->frame_number().value() << " bin " << (j << binning) << " " << histogram[j]);
            if ( histogram[highest_bin] < histogram[j] )
                highest_bin = j;
        }
        int histogram_size = 0;
        for (int i = 0; i <= highest_bin; ++i) histogram_size += histogram[i];
        DEBUG("Histogram has " << histogram_size << " pixels and highest bin is " << highest_bin);
        if ( histogram_size <= 100000 ) continue;

        double S = 0, sumweight = 0;
        mean = variance = 0;
        for (int i = 0; i <= highest_bin; ++i) {
            if ( histogram[i] == 0 ) continue;
            double newweight = sumweight + histogram[i];
            double Q = ((i << binning) - mean), R = Q * histogram[i] / newweight;
            S += sumweight * Q * R;
            mean += R;
            sumweight = newweight;
            DEBUG("Mean changed to " << mean << " and variance to " << S * (highest_bin+1) / ( (highest_bin) * sumweight) );
        }
        variance = S * (highest_bin+1) / ( (highest_bin) * sumweight);
        break;
    }

    input.dispatch( Input::RepeatInput );
    return boost::units::quantity<cs_units::camera::intensity,float>::from_value(35*sqrt(variance));
}

}
}
