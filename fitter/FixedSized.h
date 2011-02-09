#ifndef DSTORM_FITTER_CONCRETE_SIZE_H
#define DSTORM_FITTER_CONCRETE_SIZE_H

#include "Sized.h"
#include <Eigen/Core>

namespace dStorm {
namespace fitter {

template <class BaseFitter, int Width, int Height>
class FixedSized : public Sized
{
  public:
    typedef typename BaseFitter::SizeInvariants Common;
    typedef typename BaseFitter::template Specialized<Width,Height>
        ::Deriver Deriver;

  protected:
    Deriver deriver;
    Common& common;

  public:
    FixedSized(Common& common) 
        : deriver(common.constants()), common(common) {}

    inline void setSize( int width, int height ) 
        { deriver.setSize( width, height ); }
    
    int fit(
        const engine::Spot& spot, Localization* target,
        const engine::Image &image, int xl, int yl );

    const typename Deriver::Position&
    getPosition() const { return deriver.getPosition(); }
    typename Deriver::Position&
    getPosition() { return deriver.getPosition(); }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
