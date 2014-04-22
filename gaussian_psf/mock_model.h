#include <nonlinfit/plane/Disjoint.h>
#include <nonlinfit/plane/DisjointData.h>
#include "fit_window/Plane.h"

namespace dStorm {
namespace gaussian_psf {

typedef nonlinfit::plane::xs_disjoint<double,12>::type MockDataTag;
fit_window::Plane mock_data();
template <typename Expression> Expression mock_model();

}
}
