#ifndef DSTORM_PSF_NO3D_H
#define DSTORM_PSF_NO3D_H

#include "BaseExpression.h"

namespace dStorm {
namespace guf {
namespace PSF {

class No3D
: public BaseExpression
{
    template <class Num, typename Expression> friend class BaseEvaluator;
    template <class Num, typename Expression, int Size> friend class JointEvaluator;
    template <class Num, typename Expression, int Size> friend class DisjointEvaluator;
  
  public:
    using BaseExpression::Variables;

    No3D& copy( const BaseExpression& f ) { return *this = dynamic_cast<const No3D&>(f); }

    Eigen::Matrix< quantity<LengthUnit>, 2, 1 > get_sigma() const;

    typedef No3D PSF;
};

}
}
}

#endif
