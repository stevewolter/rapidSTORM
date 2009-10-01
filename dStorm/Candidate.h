#ifndef DSTORM_CANDIDATE_H
#define DSTORM_CANDIDATE_H

#include <dStorm/engine/Spot.h>
#include <memory>

namespace dStorm {
   /** A Candidate is a representation for a spot candidate.
    *  It is represented by a subpixel-precise position estimate
    *  and a strength, that is, the intensity of the smoothed
    *  image at its maximum. */
   template <typename PixelType>
   class Candidate : public std::pair<PixelType, Spot> {
      private:
        int weight;
      public:
         Candidate(const PixelType& p, const Spot &s)
            : std::pair<PixelType, Spot>(p, s), weight(1) {}

         void merge(const Candidate<PixelType> &with)
            { this->second.add(with.second); }
         inline bool operator<(const Candidate<PixelType>& other) const
 
            { if ( this->first > other.first) return true;
              else if (this->first < other.first) return false;
              else if (this->second.x() < other.second.x()) return true;
              else if (this->second.x() > other.second.x()) return false;
              else return this->second.y() < other.second.y(); }

        double x() const { return this->second.x(); }
        double y() const { return this->second.y(); }
   };
}

#endif
