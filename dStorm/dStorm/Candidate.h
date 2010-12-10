#ifndef DSTORM_CANDIDATE_H
#define DSTORM_CANDIDATE_H

#include <dStorm/Spot.h>
#include <memory>
#include <dStorm/Config.h>

namespace dStorm {
   template <typename PixelType>
   class Candidate : public std::pair<PixelType, Spot> {
      private:
        int weight;
      public:
         Candidate(const PixelType& p, const Spot &s) throw()
            : std::pair<PixelType, Spot>(p, s), weight(1) {}

         void merge(const Candidate<PixelType> &with) throw()
            { this->second.add(with.second); }
         inline bool operator<(const Candidate<PixelType>& other) const
         throw() 
            { if ( this->first > other.first) return true;
              else if (this->first < other.first) return false;
              else if (this->second.x() < other.second.x()) return true;
              else if (this->second.x() > other.second.x()) return false;
              else return this->second.y() < other.second.y(); }

        double x() const throw() { return this->second.x(); }
        double y() const throw() { return this->second.y(); }
   };
}

#endif
