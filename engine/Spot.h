#ifndef DSTORM_ENGINE_SPOT_H
#define DSTORM_ENGINE_SPOT_H

#include <Eigen/Core>
#include <boost/units/Eigen/Core>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/camera/length.hpp>

namespace dStorm {
namespace engine {
   /** A Spot is a position in an image. It can be extended by
    *  more coordinate pairs, giving the arithmethic mean of all added
    *  positions at its position. */
   class Spot { 
      private:
         float _x, _y, n_;
         typedef boost::units::quantity<boost::units::camera::length,int>
            CameraOffset;
         typedef Eigen::Matrix< CameraOffset, 2, 1 > CameraPosition;
         float x() const { return _x / n_; }
         float y() const { return _y / n_; }
      public:
         Spot(int x, int y) : _x(x), _y(y), n_(1) {}

         void add(const Spot &o) { _x += o._x; _y += o._y; n_ += o.n_; }

        bool closer_than( const Spot& o, int msx, int msy ) const {
            return std::abs(x() - o.x()) <= msx && std::abs(y() - o.y()) <= msy;
        }

        CameraPosition position() const { 
            CameraPosition rv;
            rv.x() = x() * boost::units::camera::pixel;
            rv.y() = y() * boost::units::camera::pixel;
            return rv;
        }
   };
}
}

#endif
