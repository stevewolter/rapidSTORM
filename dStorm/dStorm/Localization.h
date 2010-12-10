#ifndef DSTORM_FIT_H
#define DSTORM_FIT_H

#include <math.h>
#include <iostream>

#define _DSTORM_RESOLUTION 63000

namespace dStorm {
   class Trace;
   class Localization { 
      long int _x, _y;
      double strength;
      int _im;

      const Trace *source;

      public:
         inline Localization() throw() {}
         Localization(double x, double y, int n = -1, double s = 0,
                      const Trace *source = NULL) throw()
         : strength(s), _im(n), 
           source(source)
           { setX(x); setY(y); }

         void setX(double nx, int offset = 0) throw()
            { _x = offset * _DSTORM_RESOLUTION +
                     (int)(nx * _DSTORM_RESOLUTION); }
         void setY(double ny, int offset = 0) throw()
            { _y = offset * _DSTORM_RESOLUTION +
                     (int)(ny * _DSTORM_RESOLUTION); }

         typedef int Raster ;

         int getXLow(Raster raster = _DSTORM_RESOLUTION) const
            { return (_x / raster); }
         int getYLow(Raster raster = _DSTORM_RESOLUTION) const
            { return (_y / raster); }
         int getX(Raster raster = _DSTORM_RESOLUTION) const
            { return getXCenter(raster); }
         int getXCenter(Raster raster = _DSTORM_RESOLUTION) const 
            { return ((_x + raster/2) / raster); }
         int getY(Raster raster = _DSTORM_RESOLUTION) const
            { return getYCenter(raster); }
         int getYCenter(Raster raster = _DSTORM_RESOLUTION) const 
            { return ((_y + raster/2) / raster); }
         int getXHigh(Raster raster = _DSTORM_RESOLUTION) const
            { return ((_x + raster-1) / raster); }
         int getYHigh(Raster raster = _DSTORM_RESOLUTION) const
            { return ((_y + raster-1) / raster); }
         /* Remainders */
         float getXR(Raster raster = _DSTORM_RESOLUTION) const
            { return float(_x % raster) / (raster); }
         float getYR(Raster raster = _DSTORM_RESOLUTION) const
            { return float(_y % raster) / (raster); }

         double getPreciseX(Raster raster = _DSTORM_RESOLUTION) const 
            { return ( double(_x) / raster ); }
         double getPreciseY(Raster raster = _DSTORM_RESOLUTION) const 
            { return ( double(_y) / raster ); }
         double x(Raster raster = _DSTORM_RESOLUTION) const 
            { return ( double(_x) / raster ); }
         double y(Raster raster = _DSTORM_RESOLUTION) const 
            { return ( double(_y) / raster ); }

         void shiftX( double amount, Raster raster = _DSTORM_RESOLUTION )
            throw() { _x += (long int)round(amount * raster); }
         void shiftY( double amount, Raster raster = _DSTORM_RESOLUTION )
            throw() { _y += (long int)round(amount * raster); }
         void shiftX( long int amount )
            throw() { _x += amount; }
         void shiftY( long int amount )
            throw() { _y += amount; }

         static Raster getRaster(double resolutionEnhancement) 
            { return (int)round(_DSTORM_RESOLUTION /
                                resolutionEnhancement); }

         inline void setStrength(double val) { strength = val; }
         inline double getStrength() const throw() { return strength; }
         inline bool isStrongerThan(double val) { return strength >= val; }

         inline void setImageNumber(int num) { _im = num; }
         inline int getImageNumber() const { return _im; }
         inline int N() const throw() { return _im; }

        bool has_source_trace() const throw() { return source != NULL; }
        const Trace& get_source_trace() const throw() { return *source; }
   };

    std::ostream&
    operator<<(std::ostream &o, const Localization& loc) throw();
}

#endif
