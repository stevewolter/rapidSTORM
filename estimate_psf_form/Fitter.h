#ifndef DSTORM_FORM_FITTER_FITTER_H
#define DSTORM_FORM_FITTER_FITTER_H

#include "decl.h"
#include <memory>
#include <dStorm/engine/Image.h>
#include <dStorm/Localization_decl.h>
#include <simparm/ProgressEntry.h>

namespace dStorm {
namespace estimate_psf_form {

struct FittingVariant {
    virtual ~FittingVariant() {}

    /** Virtual constructor. 
     *  \param images Number of images that need to be added via add_image() 
     *                before fit() will be usable. */
    static std::auto_ptr<FittingVariant> create( const Config&, const input::Traits< engine::ImageStack >&, int images );
    virtual bool add_image( const engine::ImageStack& image, const Localization& position, int fluorophore ) = 0;
    virtual double collection_state() const = 0;
    virtual void fit( input::Traits< engine::ImageStack >&, simparm::ProgressEntry& ) = 0;
};

}
}

#endif
