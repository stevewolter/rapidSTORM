#include "fields_decl.h"

namespace dStorm {
namespace LocalizationFile {
namespace field {

namespace properties {

template <int Dimension> struct Spatial;
struct ZDimension;
struct Time;
struct Amplitude;
struct TwoKernelImprovement;
struct CovarianceMatrix;

}

typedef KnownWithResolution< properties::Spatial<0> >
    XCoordinate;
typedef KnownWithResolution< properties::Spatial<1> >
    YCoordinate;
typedef Known< properties::ZDimension >
    ZCoordinate;
typedef KnownWithResolution< properties::Time >
    FrameNumber;
typedef Known< properties::Amplitude > 
    Amplitude;
typedef Known< properties::TwoKernelImprovement >
    TwoKernelImprovement;
typedef Known< properties::CovarianceMatrix >
    CovarianceMatrix;

}
}
}
