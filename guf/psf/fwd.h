#ifndef DSTORM_FITTER_PSF_FWD_H
#define DSTORM_FITTER_PSF_FWD_H

#include "LengthUnit.h"

namespace dStorm {
namespace guf {
namespace PSF {

struct BaseExpression;
struct Polynomial3D;
struct No3D;
struct FixedForm;
struct FreeForm;

template <class Num> class BaseParameters;
template <class Num, class Expression> class Parameters;
template <class Num> class Parameters< Num, No3D >;
template <class Num> class Parameters< Num, Polynomial3D >;
template <class Num, class Expression, int Size> class JointEvaluator;
template <class Num, class Expression, int Size> class DisjointEvaluator;
template <class Model, class Number, class P1, class P2>
    class ReferenceEvaluator;

}
}
}

#endif
