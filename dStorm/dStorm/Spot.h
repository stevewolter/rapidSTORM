#ifndef DSTORM_SPOT_H
#define DSTORM_SPOT_H

#include <dStorm/Image.h>
#include <CImgBuffer/Image.h>

namespace dStorm {
   class Spot { 
      private:
         int _x, _y, n;
      public:
         Spot(int x, int y) : _x(x), _y(y), n(1) {}

         void add(int nx, int ny) throw() { _x += nx; _y += ny; n++; }
         void add(const Spot &o) throw() 
            { _x += o._x; _y += o._y; n+= o.n; }

         inline int x() const throw() { return _x / n; }
         inline int y() const throw() { return _y / n; }
   };
}

#endif
