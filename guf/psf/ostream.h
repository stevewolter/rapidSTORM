#ifndef DSTORM_FITTER_PSF_OSTREAM_H
#define DSTORM_FITTER_PSF_OSTREAM_H

#include "parameters.hpp"
#include <iostream>
#include <boost/units/io.hpp>

namespace dStorm {
namespace guf {
namespace PSF {

template <int Dim>
inline std::ostream& operator<<(std::ostream& o, Mean<Dim>)  { return (o << "mean" << Dim); }
template <int Dim>
inline std::ostream& operator<<(std::ostream& o, BestSigma<Dim>)  { return (o << "best sigma " << Dim); }
template <int Dim>
inline std::ostream& operator<<(std::ostream& o, DeltaSigma<Dim>)  { return (o << "delta sigma " << Dim); }
template <int Dim>
inline std::ostream& operator<<(std::ostream& o, ZOffset<Dim>)  { return (o << "z offset " << Dim); }
inline std::ostream& operator<<(std::ostream& o, Amplitude)  { return (o << "amplitude"); }
inline std::ostream& operator<<(std::ostream& o, ZPosition)  { return (o << "z position"); }
inline std::ostream& operator<<(std::ostream& o, MeanZ)  { return (o << "mean z"); }
inline std::ostream& operator<<(std::ostream& o, Prefactor)  { return (o << "prefactor"); }
inline std::ostream& operator<<(std::ostream& o, Wavelength)  { return (o << "wavelength"); }

inline std::ostream& operator<<( std::ostream& o, const BaseExpression& m ) {
    o << m( Amplitude() ) << " * " << m( Prefactor() ) << " * exp( -0.5 * [ "
      << "(x - " << m( Mean<0>() ) << ")^2 / (" << m.get_sigma()[0] << ")^2 + "
      << "(y - " << m( Mean<1>() ) << ")^2 / (" << m.get_sigma()[1] << ")^2 ] ) / "
      << 2 * M_PI * m.get_sigma()[0] * m.get_sigma()[1];
    return o;
}

}
}
}

#endif
