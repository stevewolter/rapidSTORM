#ifndef DSTORM_LOCALIZATION_FIELDS_H
#define DSTORM_LOCALIZATION_FIELDS_H

#include <boost/mpl/vector.hpp>

namespace dStorm {
namespace localization {

typedef boost::mpl::vector<
    traits::PositionX,
    traits::PositionUncertaintyX,
    traits::PositionY,
    traits::PositionUncertaintyY,
    traits::PositionZ,
    traits::PositionUncertaintyZ,
    traits::ImageNumber,
    traits::Amplitude,
    traits::PSFWidthX,
    traits::PSFWidthY,
    traits::TwoKernelImprovement,
    traits::FitResidues,
    traits::Fluorophore,
    traits::LocalBackground> Fields;

}
}

#endif
