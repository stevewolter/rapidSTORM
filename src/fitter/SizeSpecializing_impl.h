#ifndef DSTORM_FITTER_SIZESPECIALIZING_H
#define DSTORM_FITTER_SIZESPECIALIZING_H

#include "SizeSpecializing.h"
#include <dStorm/engine/Config.h>
#include <dStorm/engine/JobInfo.h>

namespace dStorm {
namespace fitter {

template <typename Type>
SizeSpecializing<Type>::SizeSpecializing( 
    const SizeSpecializingConfig& config,
    const JobInfo& info) 
: common(config, info) ,
    msx( info.config.fitWidth() / cs_units::camera::pixel ),
    msy( info.config.fitHeight() / cs_units::camera::pixel )
{
    for (int i = 0; i < MaxFitWidth-1; i++)
        for (int j = 0; j < MaxFitHeight-1; j++) {
            table[i][j] = NULL;
            factory[i][j] = NULL;
        }

    create_specializations<0>();
}

template <typename Type>
SizeSpecializing<Type>::~SizeSpecializing()
{
    for (int x = 0; x < MaxFitHeight-1; x++)
      for (int y = 0; y < MaxFitHeight-1; y++) {
        if ( table[x][y] != NULL )
            delete table[x][y];
        if ( factory[x][y] != NULL )
            delete factory[x][y];
      }
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

}
}

#endif
