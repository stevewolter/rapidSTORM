#ifndef LIBFITPP_LATTICEFITFUNCTION_H
#define LIBFITPP_LATTICEFITFUNCTION_H

#include <fit++/FitFunction.hh>
#include <cassert>
#include <algorithm>

namespace libfitpp {
    /** A LatticeFitFunction is a fit function that
     *  operates on a two-dimensional square lattice. The
     *  lattice distance is 1 in x- and y-direction,
     *  and the lattice size is given in the constructor
     *  as well as the data type.
     *
     *  The values are given using the setDataLine
     *  functions.
     **/
    template <typename DataPoint>
    class LatticeFitFunction {
      public:
        /** Destructor. */
        ~LatticeFitFunction() throw();

        void setData(const DataPoint *array, int x_step, int y_step,
                     int x_start, int x_end, int y_start, int y_end)
            { data = array; 
              this->x_step = x_step, this->y_step = y_step;
              xs = x_start, ys = y_start, xe = x_end, ye = y_end; }

        void setSize(int x_radius, int y_radius) throw() 
        { xr = x_radius; yr = y_radius; }

        DataPoint getCorner(int xc, int yc, int xfac, int yfac) throw() {
            return getPoint(std::max(std::min(xc+xfac*xr, xe), xs),
                            std::max(std::min(yc+yfac*yr, ye), ys) );
        }

      protected:
        int xr, yr, xs, ys, ye, xe;

        LatticeFitFunction() throw();

        inline DataPoint getPoint(int x, int y) 
            const throw() 
            { assert( x >= xs && x <= xe && y <= ye && y >= ys);
              return data[x*x_step + y*y_step]; }

      private:
        const DataPoint* data;
        int x_step, y_step;
    };
}

#endif
