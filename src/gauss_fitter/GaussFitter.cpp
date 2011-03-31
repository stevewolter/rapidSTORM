#include "debug.h"
#include "GaussFitter_impl.h"
#include <CImg.h>

namespace dStorm {
namespace engine {

template <bool Free_Sigmas, bool RA, bool Corr>
int GaussFitter<Free_Sigmas, RA, Corr>::
fitSpot( const Spot& spot, const Image& image, Localization* target )
{
    int xc = int(round(spot.x())), yc = int(round(spot.y()));
    int xl = std::max( xc-msx, 0 ), yl = std::max( yc-msy, 0 );
#if cimg_version > 129
    int xh = std::min<int>( xc+msx, image.width() - 1 ),
        yh = std::min<int>( yc+msy, image.height() - 1 );
#else
    int xh = std::min<int>( xc+msx, image.width - 1 ),
        yh = std::min<int>( yc+msy, image.height - 1 );
#endif

    int xs = (xh - xl) + 1, ys = (yh - yl) + 1;
    DEBUG("Searching Gaussian fitter for size " << xs << " " << yh << " with maximum size " << MaxFitWidth << " " << MaxFitHeight);
    BaseTableEntry *e;
    if ( xs >= MaxFitWidth || ys >= MaxFitHeight || xs < 1 || ys < 1 ) {
        DEBUG("Fitter out of table range, making new fitter");
        if ( dynamic_fitter.get() == NULL )
            dynamic_fitter.reset( 
                dynamic_fitter_factory->factory(common) );
        dynamic_fitter->setSize( xs, ys );
        e = dynamic_fitter.get();
    } else {
        DEBUG("Looking up fitter at " << xs-1 << " " << ys-1);
        e = table[xs-1][ys-1];
        if ( e == NULL ) {
            if ( factory[xs-1][ys-1] != NULL ) {
                DEBUG("Using factory for " << xs-1 << " " << ys-1 << " with address " << factory[xs-1][ys-1]);
                e = factory[xs-1][ys-1]->factory(common);
                DEBUG("Made static fitter at address " << e);
            } else {
                DEBUG("Making new dynamic fitter");
                BaseTableEntry *d = 
                    dynamic_fitter_factory->factory(common);
                d->setSize(xs, ys);
                e = d;
                DEBUG("Made dynamic fitter at address " << e);
            }
            DEBUG("Storing fitter at " << xs-1 << " " << ys-1);
            table[xs-1][ys-1] = e;
        }
    }
    return e->fit( spot, target, image, xl, yl );
}

template int GaussFitter<true,true,true>::fitSpot
    (const Spot&, const Image&, Localization*);
template int GaussFitter<true,false,true>::fitSpot
    (const Spot&, const Image&, Localization*);
template int GaussFitter<false,true,true>::fitSpot
    (const Spot&, const Image&, Localization*);
template int GaussFitter<false,false,true>::fitSpot
    (const Spot&, const Image&, Localization*);
template int GaussFitter<false,true,false>::fitSpot
    (const Spot&, const Image&, Localization*);
template int GaussFitter<false,false,false>::fitSpot
    (const Spot&, const Image&, Localization*);
template int GaussFitter<true,true,false>::fitSpot
    (const Spot&, const Image&, Localization*);
template int GaussFitter<true,false,false>::fitSpot
    (const Spot&, const Image&, Localization*);
}
}
