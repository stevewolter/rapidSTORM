#include <nonlinfit/plane/Disjoint.h>
#include <nonlinfit/plane/DisjointData.hpp>
#include "LengthUnit.h"
#include "dejagnu.h"

namespace dStorm {
namespace guf {
namespace PSF {

typedef nonlinfit::plane::xs_disjoint<double,LengthUnit,12>::type MockDataTag;
MockDataTag::Data mock_data();

template <typename Expression> Expression mock_model();

void check_zhuang_evaluator( TestState& );
void check_no3d_evaluator( TestState& );
void check_spline_evaluator( TestState& );

}
}
}
