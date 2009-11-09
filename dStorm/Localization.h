#ifndef DSTORM_FIT_H
#define DSTORM_FIT_H

#include <math.h>
#include <iostream>

#include "output/Trace_decl.h"

#define _DSTORM_RESOLUTION 63000

namespace dStorm {
   class Localization { 
      double _x, _y;
      double strength;
      int _im;
      double parab;

      const output::Trace *source;

      public:
         inline Localization() {}
         Localization(double x, double y, int n = -1, double s = 0,
                      const output::Trace *source = NULL, double parab = 0)

         : _x(x), _y(y), strength(s), _im(n), 
           parab(parab), source(source) {}

         void setX(double nx, int offset = 0)
            { _x = nx + offset; }
         void setY(double ny, int offset = 0)
            { _y = ny + offset; }

         typedef float Raster ;

         int getXLow(Raster raster = _DSTORM_RESOLUTION) const
            { return floor(_x * raster); }
         int getYLow(Raster raster = _DSTORM_RESOLUTION) const
            { return floor(_y * raster); }
         int getXCenter(Raster raster = _DSTORM_RESOLUTION) const 
            { return round(_x * raster); }
         int getYCenter(Raster raster = _DSTORM_RESOLUTION) const 
            { return round(_y * raster); }
         int getXHigh(Raster raster = _DSTORM_RESOLUTION) const
            { return ceil(_x * raster); }
         int getYHigh(Raster raster = _DSTORM_RESOLUTION) const
            { return ceil(_y * raster); }
         float getXR(Raster raster = _DSTORM_RESOLUTION) const
            { return _x * raster - floor(_x * raster); }
         float getYR(Raster raster = _DSTORM_RESOLUTION) const
            { return _y * raster - floor(_y * raster); }

         double getPreciseX() const { return _x; }
         double getPreciseY() const { return _y; }
         double x() const { return _x; }
         double y() const { return _y; }

         void shiftX( double amount ) { _x += amount; }
         void shiftY( double amount ) { _y += amount; }

         static Raster getRaster(double resolutionEnhancement) 
            { return resolutionEnhancement; }

         inline void setStrength(double val) { strength = val; }
         inline double getStrength() const { return strength; }
         inline bool isStrongerThan(double val) { return strength >= val; }

         inline void setImageNumber(int num) { _im = num; }
         inline int getImageNumber() const { return _im; }
         inline int N() const { return _im; }

        bool has_source_trace() const { return source != NULL; }
        const output::Trace& get_source_trace() const { return *source; }

         inline double parabolicity() const { return parab; }
   };

    std::ostream&
    operator<<(std::ostream &o, const Localization& loc);
}

#include <dStorm/data-c++/Traits.h>

namespace data_cpp {

template <> class Traits<dStorm::Localization>
    : public Traits<int> {};

}

#endif
