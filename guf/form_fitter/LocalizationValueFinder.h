#ifndef DSTORM_GUF_FORM_FITTER_LOCALIZATIONVALUEFINDER_H
#define DSTORM_GUF_FORM_FITTER_LOCALIZATIONVALUEFINDER_H

#include "guf/psf/Polynomial3D.h"
#include "guf/psf/No3D.h"
#include "guf/constant_background.hpp"
#include <memory>
#include <dStorm/engine/JobInfo_decl.h>
#include <dStorm/traits/optics.h>
#include <dStorm/Localization_decl.h>

namespace dStorm {
namespace form_fitter {

struct LocalizationValueFinder 
{
    struct application;
    std::auto_ptr<application> appl_;

    template <typename Type> void find_values_( Type& );

  public:
    LocalizationValueFinder( 
        const dStorm::engine::JobInfo& info, const dStorm::traits::Optics& plane,
        const Localization& parent, size_t plane_number );

    void find_values( guf::PSF::Polynomial3D& z ) { find_values_(z); }
    void find_values( guf::PSF::No3D& z ) { find_values_(z); }
    void find_values( constant_background::Expression& z ) { find_values_(z); }
};

}
}

#endif
