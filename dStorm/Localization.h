#ifndef DSTORM_FIT_H
#define DSTORM_FIT_H

#include <math.h>
#include <iostream>
#include <Eigen/Core>

#include "camera_units.h"
#include <boost/units/quantity.hpp>
#include "output/Trace_decl.h"
#include "units_Eigen_traits.h"

#define _DSTORM_RESOLUTION 63000

namespace dStorm {
    class Localization { 
      public:
        static const int Dim = 2;
        typedef boost::units::quantity<camera::length,float>
            Coord;
        typedef 
            Eigen::Matrix<Coord, Dim, 1, Eigen::DontAlign>
            Position;
        typedef
            Eigen::Matrix< 
                boost::units::quantity<camera::area,float>,
                Dim, Dim, Eigen::DontAlign>
            Matrix;

      private:
        Position _position;
        float _strength;
        Matrix _fit_covariance_matrix;
        float _two_kernel_improvement;

        const output::Trace *source;
        int _im;

      public:
        inline Localization( const Position& position, 
                      float strength );

        Position& position() { return _position; }
        const Position& position() const { return _position; }

        Coord& x() { return _position.x(); }
        Coord& y() { return _position.y(); }
        Coord x() const { return _position.x(); }
        Coord y() const { return _position.y(); }

        float& strength() { return _strength; }
        const float& strength() const { return _strength; }
        float getStrength() const { return _strength; }

        Matrix& fit_covariance_matrix() { return _fit_covariance_matrix; }
        const Matrix& fit_covariance_matrix() const
            { return _fit_covariance_matrix; }

        float& two_kernel_improvement() { return _two_kernel_improvement; }
        const float& two_kernel_improvement() const 
            { return _two_kernel_improvement; }

        inline void setImageNumber(int num) { _im = num; }
        inline int getImageNumber() const { return _im; }
        inline int N() const { return _im; }

        bool has_source_trace() const { return source != NULL; }
        const output::Trace& get_source_trace() const { return *source; }
        void set_source_trace( const output::Trace& t) { source = &t; }
        void unset_source_trace() { source = NULL; }
   };

    std::ostream&
    operator<<(std::ostream &o, const Localization& loc);

    Localization::Localization( 
        const Position& position,
        float strength
    )
        : _position(position), _strength(strength),
          _fit_covariance_matrix( Matrix::Constant( -1 * camera::pixel * camera::pixel ) ),
            _two_kernel_improvement(0),
            source(NULL), _im(-1) {}
              
}

#include <dStorm/data-c++/Traits.h>

namespace data_cpp {

template <> class Traits<dStorm::Localization>
    : public Traits<int> {};

}

#endif
