#include <nonlinfit/plane/Disjoint.h>
#include <nonlinfit/plane/DisjointData.hpp>

namespace dStorm {
namespace gaussian_psf {

typedef nonlinfit::plane::xs_disjoint<double,12>::type MockDataTag;
MockDataTag::Data mock_data();
template <typename Expression> Expression mock_model();

}
}
