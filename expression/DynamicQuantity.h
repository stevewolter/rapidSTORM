#ifndef DSTORM_EXPRESSION_DYNAMICQUANTITY_H
#define DSTORM_EXPRESSION_DYNAMICQUANTITY_H

#include <Eigen/Core>

namespace dStorm {
namespace expression {

namespace BaseUnits {
    enum BaseUnit {
        Pixel,
        Meter,
        Frame,
        Second,
        ADCount,
        Number
    };
}


struct DynamicUnit : public Eigen::Matrix<double, BaseUnits::Number, 1, Eigen::DontAlign> {
    static std::string unit_names[BaseUnits::Number];
    typedef Eigen::Matrix<double, BaseUnits::Number, 1, Eigen::DontAlign> Base;
    DynamicUnit() : Base(Base::Zero()) {}
    DynamicUnit( const Base& b ) : Base(b) {}
    template <typename Type>
    explicit DynamicUnit( const Type& t ) : Base(t) {}
    static DynamicUnit Dimensionless() { return DynamicUnit(); }
};

struct DynamicQuantity {
    double value;
    DynamicUnit unit;
    DynamicQuantity() {}
    DynamicQuantity(double v, DynamicUnit u) : value(v), unit(u) {}
};

}
}

#endif
