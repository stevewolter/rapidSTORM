#ifndef DSTORM_FITTER_SIZESPECIALIZING_H
#define DSTORM_FITTER_SIZESPECIALIZING_H

#include "debug.h"
#include "SizeSpecializing.h"
#include <dStorm/engine/Config.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/engine/Spot.h>

namespace dStorm {
namespace fitter {

using engine::Spot;
using engine::Image;

template <typename Type>
SizeSpecializing<Type>::SizeSpecializing( 
    const Common& common, const engine::JobInfo& info )
: common(common) ,
    msx( info.config.fitWidth() / camera::pixel ),
    msy( info.config.fitHeight() / camera::pixel )
{
    for (int i = 0; i < MaxFitWidth-1; i++)
        for (int j = 0; j < MaxFitHeight-1; j++) {
            table[i][j] = NULL;
        }

    create_specializations<0>();
}

template <typename Type>
SizeSpecializing<Type>::~SizeSpecializing()
{
    DEBUG( "Destructing size specializing fitter" );
    for (int x = 0; x < MaxFitHeight-1; x++)
      for (int y = 0; y < MaxFitHeight-1; y++) {
        if ( table[x][y] != NULL )
            delete table[x][y];
      }
    DEBUG( "Destructed size specializing fitter table" );
    dynamic_fitter.reset( NULL );
    DEBUG( "Destructed dynamic size fitter" );
}

template <typename Type>
int SizeSpecializing<Type>::
fitSpot( const Spot& spot, const Image& image, Localization* target )
{
    int xc = int(round(spot.x())), yc = int(round(spot.y()));
    int xl = std::max( xc-msx, 0 ), yl = std::max( yc-msy, 0 );
    int xh = std::min<int>( xc+msx, image.width().value() - 1 ),
        yh = std::min<int>( yc+msy, image.height().value() - 1 );

    int xs = (xh - xl) + 1, ys = (yh - yl) + 1;
    DEBUG("Searching Gaussian fitter for size " << xs << " " << ys << " with maximum size " << MaxFitWidth << " " << MaxFitHeight
          << " and mask size " << msx << " " << msy);
    BaseTableEntry *e;
    if ( xs >= MaxFitWidth || ys >= MaxFitHeight || xs < 1 || ys < 1 ) {
        DEBUG("Fitter out of table range, making new fitter");
        if ( dynamic_fitter.get() == NULL )
            dynamic_fitter = make_unspecialized_fitter();
        dynamic_fitter->setSize( xs, ys );
        e = dynamic_fitter.get();
    } else {
        DEBUG("Looking up fitter at " << xs-1 << " " << ys-1);
        e = table[xs-1][ys-1];
        if ( e == NULL ) {
            DEBUG("Making new dynamic fitter");
            std::auto_ptr<BaseTableEntry> d
                = make_unspecialized_fitter();
            d->setSize(xs, ys);
            e = d.release();
            DEBUG("Made dynamic fitter at address " << e);
            DEBUG("Storing fitter at " << xs-1 << " " << ys-1);
            table[xs-1][ys-1] = e;
        }
    }
    return e->fit( spot, target, image, xl, yl );
}

}
}

#endif
