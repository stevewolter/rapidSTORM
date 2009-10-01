#ifndef DSTORM_SPOT_H
#define DSTORM_SPOT_H

#include <dStorm/engine/Image.h>

namespace dStorm {
   /** A Spot is a position in an image. It can be extended by
    *  more coordinate pairs, giving the arithmethic mean of all added
    *  positions at its position. */
   class Spot { 
      private:
         int _x, _y, n;
      public:
         Spot(int x, int y) : _x(x), _y(y), n(1) {}

         void add(int nx, int ny) { _x += nx; _y += ny; n++; }
         void add(const Spot &o) 
            { _x += o._x; _y += o._y; n+= o.n; }

         inline int x() const { return _x / n; }
         inline int y() const { return _y / n; }
   };
}

#endif
