#include "MedianSmoother.h"
#include <CImgBuffer/Image.h>

using namespace cimg_library;
using namespace dStorm;

int main() {
    int w = 128, h = 128;
    dStorm::Image im(w, h, 0);
    double sz = 0.6;

    for (int i = 0; i < w*h; i++)
        im( i % w, i / w ) = i/w;

    dStorm::Config conf;
    conf.sigma_x = sz;
    conf.sigma_y = sz;

    dStorm::MedianSmoother smoothre(conf, w, h);
    smoothre.prepare(im);

    CImg<SmoothedPixel> handSmoothed = im;
    handSmoothed.blur_median( 2*conf.x_maskSize()+1 );
    for (unsigned int y = conf.x_maskSize(); y < im.height - conf.x_maskSize(); y++)
      for (unsigned int x = conf.x_maskSize(); x < im.width - conf.x_maskSize(); x++)
        if ( (smoothre.getSmoothed())(x,y) != handSmoothed(x,y) )
            cerr << "Error: " << x << " " << y << " " 
                 << (smoothre.getSmoothed())(x,y) << " " << handSmoothed(x,y)
                 << endl;

}
