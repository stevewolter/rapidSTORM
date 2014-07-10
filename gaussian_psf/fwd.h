#ifndef DSTORM_FITTER_PSF_FWD_H
#define DSTORM_FITTER_PSF_FWD_H

namespace dStorm {
namespace gaussian_psf {

class BaseExpression;
class No3D;
class DepthInfo3D;
class FixedForm;
class FreeForm;

template <class Num> class BaseParameters;
template <class Num, class Expression> class Parameters;
template <class Num> class Parameters< Num, No3D >;
template <class Num, class Expression, int Size> class JointEvaluator;
template <class Num, class Expression, int Size> class DisjointEvaluator;
template <class Model, class Number, class P1, class P2>
    class ReferenceEvaluator;

}
}

#endif
