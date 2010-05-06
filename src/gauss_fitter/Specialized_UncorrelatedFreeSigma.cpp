#include "GaussFitter_Specialized.h"
#include <fit++/Exponential2D_Uncorrelated_Derivatives.hh>
#include "GaussFitter_ResidueAnalysis.h"

namespace dStorm {
namespace engine {

template <bool A, bool B, bool C>
template <int Level>
void GaussFitter<A,B,C>::
create_specializations()
{
    dynamic_fitter_factory.reset( 
        new TableEntryMaker<Eigen::Dynamic,Eigen::Dynamic>() );
}

template void GaussFitter<true,false,false>::create_specializations<0>();
template void GaussFitter<true,true,false>::create_specializations<0>();

template class GaussFitter<true,true,false>;
template class GaussFitter<true,false,false>;

}
}
