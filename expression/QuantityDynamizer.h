#ifndef DSTORM_EXPRESSION_QUANTITY_READER_H
#define DSTORM_EXPRESSION_QUANTITY_READER_H

#include "DynamicQuantity.h"

namespace dStorm {
namespace expression {

template <typename Quantity>
class QuantityDynamizer
{
    double scale;
    DynamicUnit unit;

public:
    QuantityDynamizer();
    Quantity operator()( const DynamicQuantity& ) const;
    DynamicQuantity operator()( const Quantity& ) const;
    DynamicQuantity from_value( typename Quantity::value_type v ) const
        { return DynamicQuantity(v, unit); }
};

}
}

#endif
