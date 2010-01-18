#include "engine/GaussFitter_impl.h"
#include <CImg.h>

namespace dStorm {
namespace engine {

template <bool Free_Sigmas, bool RA, bool Corr>
int GaussFitter<Free_Sigmas, RA, Corr>::
fitSpot( const Spot& spot, const Image& image, Localization* target )
{
    int xc = int(round(spot.x())), yc = int(round(spot.y()));
    int xl = std::max( xc-msx, 0 ), yl = std::max( yc-msy, 0 );
    int xh = std::min<int>( xc+msx, image.width - 1 ),
        yh = std::min<int>( yc+msy, image.height - 1 );

    int xs = (xh - xl) + 1, ys = (yh - yl) + 1;
    BaseTableEntry *e;
    if ( xs >= MaxFitWidth || ys >= MaxFitHeight ) {
        if ( dynamic_fitter.get() == NULL )
            dynamic_fitter.reset( 
                dynamic_fitter_factory->factory(common) );
        dynamic_fitter->setSize( xs, ys );
        e = dynamic_fitter.get();
    } else {
        e = table[xs-1][ys-1];
        if ( e == NULL ) {
            if ( factory[xs-1][ys-1] != NULL )
                e = factory[xs-1][ys-1]->factory(common);
            else {
                BaseTableEntry *d = 
                    dynamic_fitter_factory->factory(common);
                d->setSize(xs, ys);
                e = d;
            }
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
}
}
