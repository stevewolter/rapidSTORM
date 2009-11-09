#include <fit++/Exponential2D_Uncorrelated_Derivatives.hh>
#include "GaussFitter_Specialized.h"

namespace dStorm {
namespace engine {

template <>
template <>
void GaussFitter<false,false,false>::
create_specializations<0>()
{
    this->fill_specialization_array<3,6>();
    //this->fill_specialization_array<8,13,13,13,13,13>();
    //this->fill_specialization_array<13,13,13,8,13,13>();
    dynamic_fitter_factory.reset( 
        new TableEntryMaker<Eigen::Dynamic,Eigen::Dynamic>() );
}

template class GaussFitter<false,false,false>;

}
}
