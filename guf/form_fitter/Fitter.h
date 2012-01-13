#ifndef DSTORM_FORM_FITTER_FITTER_H
#define DSTORM_FORM_FITTER_FITTER_H

#include "decl.h"
#include <memory>
#include <dStorm/ImageTraits.h>
#include <dStorm/engine/Image.h>
#include <dStorm/Localization_decl.h>

namespace dStorm {
namespace form_fitter {

struct FittingVariant {
    virtual ~FittingVariant() {}

    /** Virtual constructor. 
     *  \param images Number of images that need to be added via add_image() 
     *                before fit() will be usable. */
    static std::auto_ptr<FittingVariant> create( const Config&, const input::Traits< engine::Image >&, int images );
    virtual bool add_image( const engine::Image& image, const Localization& position, int fluorophore ) = 0;
    virtual void fit( input::Traits< engine::Image >& ) = 0;
};

}
}

#endif
