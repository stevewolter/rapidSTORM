#ifndef FITPP_BITFIELDCONSTRUCTOR_HH
#define FITPP_BITFIELDCONSTRUCTOR_HH
#include <Eigen/Core>

namespace fitpp {
    template <int i> struct Bits {
        static const int Count = (i % 2) + Bits< i/2 >::Count; 
    };
    template <> struct Bits<0> { static const int Count = 0; };

    template <int Field, int Number> struct Nth_One_Bit {
        static const int Position =
                Nth_One_Bit<Field/2,Number-(Field%2)>::Position + 1;
    };

    template <int Field> struct Nth_One_Bit<Field,-1> {
        static const int Position = -1;
    };
}
#endif
