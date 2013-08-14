#ifndef DSTORM_FITTER_PSF_FWD_H
#define DSTORM_FITTER_PSF_FWD_H

namespace dStorm {
namespace gaussian_psf {

struct BaseExpression;
struct No3D;
struct DepthInfo3D;
struct FixedForm;
struct FreeForm;

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
