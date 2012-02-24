#ifndef DSTORM_TRAITS_DEPTHINFO_H
#define DSTORM_TRAITS_DEPTHINFO_H

namespace dStorm {
namespace traits {

struct No3D {};

struct Zhuang3D {
    typedef units::power_typeof_helper< 
            units::si::length,
            units::static_rational<-1> >::type Unit; 
    typedef Eigen::Matrix< units::quantity< Unit >, 2, 1, Eigen::DontAlign > 
        Widening;
    Widening widening;
};

typedef boost::variant< traits::Zhuang3D, traits::No3D > DepthInfo;

}
}

#endif
