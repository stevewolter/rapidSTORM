#ifndef DSTORM_GUF_FORM_FITTER_LOCALIZATIONVALUEFINDER_H
#define DSTORM_GUF_FORM_FITTER_LOCALIZATIONVALUEFINDER_H

#include "gaussian_psf/No3D.h"
#include "constant_background/model.hpp"
#include <memory>
#include "engine/JobInfo_decl.h"
#include "traits/optics.h"
#include "Localization_decl.h"

namespace dStorm {
namespace estimate_psf_form {

struct LocalizationValueFinder 
{
    struct application;
    std::auto_ptr<application> appl_;

    template <typename Type> void find_values_( Type& );

  public:
    LocalizationValueFinder( 
        const int fluorophore, const dStorm::traits::Optics& plane,
        const Localization& parent, size_t plane_number );

    void find_values( gaussian_psf::DepthInfo3D& z );
    void find_values( gaussian_psf::No3D& z ) { find_values_(z); }
    void find_values( constant_background::Expression& z ) { find_values_(z); }
};

}
}

#endif
