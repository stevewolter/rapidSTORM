#include <nonlinfit/plane/Disjoint.h>
#include <nonlinfit/plane/DisjointData.hpp>
#include "LengthUnit.h"

namespace dStorm {
namespace gaussian_psf {

typedef nonlinfit::plane::xs_disjoint<double,LengthUnit,12>::type MockDataTag;
MockDataTag::Data mock_data();
template <typename Expression> Expression mock_model();

}
}
