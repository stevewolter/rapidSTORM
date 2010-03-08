#include "fields_decl.h"

namespace dStorm {
namespace LocalizationFile {
namespace field {

namespace properties {

template <int Dimension> struct Spatial;
struct Time;
struct Amplitude;
struct TwoKernelImprovement;

}

typedef KnownWithResolution< properties::Spatial<0> >
    XCoordinate;
typedef KnownWithResolution< properties::Spatial<1> >
    YCoordinate;
typedef KnownWithResolution< properties::Spatial<2> >
    ZCoordinate;
typedef KnownWithResolution< properties::Time >
    FrameNumber;
typedef Known< properties::Amplitude > 
    Amplitude;
typedef Known< properties::TwoKernelImprovement >
    TwoKernelImprovement;

}
}
}
