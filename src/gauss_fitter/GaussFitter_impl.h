#ifndef DSTORM_ENGINE_GAUSSFITTER_IMPL_H
#define DSTORM_ENGINE_GAUSSFITTER_IMPL_H

#include "GaussFitter.h"
#include <dStorm/engine/Config.h>
#include <dStorm/engine/JobInfo.h>

namespace dStorm {
namespace engine {

template <bool FS, bool RS, bool Corr>
GaussFitter<FS,RS,Corr>::GaussFitter( 
    const GaussFitterConfig& config,
    const JobInfo& info) 
: common(config, info) ,
    msx( info.config.fitWidth() ), msy( info.config.fitHeight() )
{
    for (int i = 0; i < MaxFitWidth-1; i++)
        for (int j = 0; j < MaxFitHeight-1; j++) {
            table[i][j] = NULL;
            factory[i][j] = NULL;
        }

    create_specializations<0>();
}

template <bool FS, bool RS, bool Corr>
GaussFitter<FS,RS,Corr>::~GaussFitter()
{
    for (int x = 0; x < MaxFitHeight-1; x++)
      for (int y = 0; y < MaxFitHeight-1; y++) {
        if ( table[x][y] != NULL )
            delete table[x][y];
        if ( factory[x][y] != NULL )
            delete factory[x][y];
      }
}

}
}

#endif
