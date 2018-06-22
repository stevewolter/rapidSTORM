#ifndef DSTORM_GUF_PSF_FIXED_FORM_H
#define DSTORM_GUF_PSF_FIXED_FORM_H

#include "gaussian_psf/parameters.h"

namespace dStorm {
namespace gaussian_psf {

struct FixedForm
{
    template <typename Type> struct apply { typedef boost::mpl::false_ type; };
};
template <int Dim> struct FixedForm::apply< Mean<Dim> > {typedef boost::mpl::true_ type; };
template <> struct FixedForm::apply< MeanZ > {typedef boost::mpl::true_ type; };
template <> struct FixedForm::apply< Amplitude > {typedef boost::mpl::true_ type; };

}
}

#endif
