#ifndef DSTORM_GUF_PSF_FREEFORM_H
#define DSTORM_GUF_PSF_FREEFORM_H

#include "gaussian_psf/parameters.h"

namespace dStorm {
namespace gaussian_psf {

struct FreeForm
{
    template <typename Type> struct apply { typedef boost::mpl::false_ type; };
};
template <int Dim> struct FreeForm::apply< Mean<Dim> > { typedef boost::mpl::true_ type; };
template <> struct FreeForm::apply< MeanZ > { typedef boost::mpl::true_ type; };
template <> struct FreeForm::apply< Amplitude > { typedef boost::mpl::true_ type; };
template <int Dim> struct FreeForm::apply< BestSigma<Dim> > { typedef boost::mpl::true_ type; };

}
}

#endif
