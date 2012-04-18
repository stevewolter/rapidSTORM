#include <Eigen/StdVector>
#include "Input.h"
#include <boost/units/Eigen/Array>
#include "guf/guf/TransformedImage.hpp"

namespace dStorm {
namespace form_fitter {

Input::Input(const Config& c, const output::Output::Announcement& a, guf::Spot width )
: number_of_spots( c.number_of_spots() )
{
    traits = a.input_image_traits;
    if ( traits.get() == NULL ) {
            throw std::runtime_error("The PSF form fitter output needs "
                "access to the raw source images, but these are not supplied "
                "at its current position in the output tree.");
    }
    fluorophore_count = std::max(1, int(traits->fluorophores.size()));
    transforms.clear();
    for (int i = 0; i < traits->plane_count(); ++i)
        transforms.push_back( new Transformed(width, traits->plane(i) ) );
}

}
}
