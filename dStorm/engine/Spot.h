#ifndef DSTORM_ENGINE_SPOT_H
#define DSTORM_ENGINE_SPOT_H

#include <Eigen/Core>
#include "../units_Eigen_traits.h"
#include <boost/units/quantity.hpp>
#include <boost/units/systems/camera/length.hpp>

namespace dStorm {
namespace engine {
   /** A Spot is a position in an image. It can be extended by
    *  more coordinate pairs, giving the arithmethic mean of all added
    *  positions at its position. */
   class Spot { 
      private:
         int _x, _y, n;
         typedef boost::units::quantity<boost::units::camera::length,int>
            CameraOffset;
         typedef Eigen::Matrix< CameraOffset, 2, 1 > CameraPosition;
      public:
         Spot(int x, int y) : _x(x), _y(y), n(1) {}

         void add(int nx, int ny) { _x += nx; _y += ny; n++; }
         void add(const Spot &o) 
            { _x += o._x; _y += o._y; n+= o.n; }

         inline int x() const { return _x / n; }
         inline int y() const { return _y / n; }

         operator CameraPosition() const { 
            CameraPosition rv;
            rv.x() = x() * boost::units::camera::pixel;
            rv.y() = y() * boost::units::camera::pixel;
            return rv;
        }
   };
}
}

#endif
