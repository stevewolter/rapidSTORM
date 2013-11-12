#ifndef DSTORM_LOCALIZATION_FIELDS_H
#define DSTORM_LOCALIZATION_FIELDS_H

#include <boost/mpl/vector.hpp>

#include "dStorm/traits/image_number.h"
#include "dStorm/traits/position.h"
#include "dStorm/traits/position_uncertainty.h"
#include "dStorm/traits/amplitude.h"
#include "dStorm/traits/psf_width.h"
#include "dStorm/traits/two_kernel_improvement.h"
#include "dStorm/traits/residues.h"
#include "dStorm/traits/fluorophore.h"
#include "dStorm/traits/local_background.h"

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
