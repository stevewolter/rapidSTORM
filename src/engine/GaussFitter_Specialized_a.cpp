#include <fit++/Exponential2D_Uncorrelated_Derivatives.hh>
#include "GaussFitter_Specialized.h"
#include "GaussFitter_ResidueAnalysis.h"

namespace dStorm {
namespace engine {

template <>
template <>
void GaussFitter<false,true,false>::create_specializations<1>()
{
#if 0
    this->fill_specialization_array<3,4>();
#endif
    //this->fill_specialization_array<8,13,13,13,13,13>();
    //this->fill_specialization_array<13,13,13,8,13,13>();
}

template class GaussFitter<false,true,false>;

}
}
