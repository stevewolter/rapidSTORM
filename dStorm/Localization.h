#ifndef DSTORM_FIT_H
#define DSTORM_FIT_H

#include <math.h>
#include <iostream>
#include <Eigen/Core>

#include <boost/units/quantity.hpp>
#include <cs_units/camera/length.hpp>
#include <cs_units/camera/area.hpp>
#include <cs_units/camera/intensity.hpp>
#include <dStorm/units/frame_count.h>
#include "output/Trace_decl.h"
#include "units_Eigen_traits.h"
#include <dStorm/units/nanolength.h>

#define _DSTORM_RESOLUTION 63000

namespace dStorm {
    class Localization { 
      public:
        static const int Dim = 2;
        typedef boost::units::quantity<cs_units::camera::length,float>
            Coord;
        typedef 
            Eigen::Matrix<Coord, Dim, 1, Eigen::DontAlign>
            Position;
        typedef 
            boost::units::quantity<boost::units::si::nanolength,float>
            ZPosition;
        typedef
            Eigen::Matrix< 
                boost::units::quantity<cs_units::camera::area,float>,
                Dim, Dim, Eigen::DontAlign>
            Matrix;
        typedef boost::units::quantity<cs_units::camera::intensity,float>
            Amplitude;

      private:
        Position _position, _uncertainty;
        ZPosition _zposition, _zuncertainty;
        Amplitude _strength;
        Matrix _fit_covariance_matrix;
        float _two_kernel_improvement;

        const output::Trace *source;
        frame_index _im;

      public:
        Localization() {}
        inline Localization( const Position& position, 
                      Amplitude strength );

        Position& position() { return _position; }
        const Position& position() const { return _position; }
        Position& uncertainty() { return _uncertainty; }
        const Position& uncertainty() const { return _uncertainty; }
        ZPosition& zposition() { return _zposition; }
        const ZPosition& zposition() const { return _zposition; }
        ZPosition& zuncertainty() { return _zuncertainty; }
        const ZPosition& zuncertainty() const { return _zuncertainty; }

        Coord& x() { return _position.x(); }
        Coord& y() { return _position.y(); }
        Coord x() const { return _position.x(); }
        Coord y() const { return _position.y(); }

        Amplitude& strength() { return _strength; }
        const Amplitude& strength() const { return _strength; }
        Amplitude getStrength() const { return _strength; }

        Matrix& fit_covariance_matrix() { return _fit_covariance_matrix; }
        const Matrix& fit_covariance_matrix() const
            { return _fit_covariance_matrix; }

        float& two_kernel_improvement() { return _two_kernel_improvement; }
        const float& two_kernel_improvement() const 
            { return _two_kernel_improvement; }

        inline void setImageNumber(frame_index num) { _im = num; }
        inline frame_index getImageNumber() const { return _im; }
        inline frame_index N() const { return _im; }
        inline frame_index frame_number() const { return _im; }

        bool has_source_trace() const { return source != NULL; }
        const output::Trace& get_source_trace() const { return *source; }
        void set_source_trace( const output::Trace& t) { source = &t; }
        void unset_source_trace() { source = NULL; }
   };

    std::ostream&
    operator<<(std::ostream &o, const Localization& loc);

    Localization::Localization( 
        const Position& position,
        Amplitude strength
    )
        : _position(position), _strength(strength),
          _fit_covariance_matrix( Matrix::Constant( Matrix::Scalar::from_value(-1) ) ),
            _two_kernel_improvement(0),
            source(NULL), _im( frame_index::from_value(-1) ) {}
              
}

#include <dStorm/data-c++/Traits.h>

namespace data_cpp {

template <> class Traits<dStorm::Localization>
    : public Traits<int> {};

}

#endif
