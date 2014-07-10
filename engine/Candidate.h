#ifndef DSTORM_ENGINE_CANDIDATE_H
#define DSTORM_ENGINE_CANDIDATE_H

#include "engine/Candidate_decl.h"
#include "engine/Spot.h"
#include <memory>

namespace dStorm {
namespace engine {
/** A Candidate is a representation for a spot candidate.
*  It is represented by a subpixel-precise position estimate
*  and a strength, that is, the intensity of the smoothed
*  image at its maximum. */
template <typename PixelType>
class Candidate {
  private:
    Spot spot_;
    PixelType strength_;
  public:
    Candidate(const PixelType& p, const Spot &s)
      : spot_(s), strength_(p) {}

    void merge(const Candidate<PixelType> &with) { 
        assert( strength_ == with.strength_ );
        spot_.add(with.spot_); 
    }

    const Spot& spot() const { return spot_; }
    const PixelType& strength() const { return strength_; }
    class decreasing_strength;
};

template <class PixelType>
class Candidate<PixelType>::decreasing_strength
: public std::binary_function< Candidate, Candidate, bool > 
{
  public:
    bool operator()( const Candidate& a, const Candidate& b ) const
        { return a.strength_ > b.strength_; }
};

}
}

#endif
