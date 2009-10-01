#include "GaussFitter_Specialized.h"
#include <fit++/Exponential2D_Correlated_Derivatives.hh>
#include "GaussFitter_ResidueAnalysis.h"

namespace dStorm {

template <bool A, bool B, bool C>
template <int Level>
void GaussFitter<A,B,C>::
create_specializations()
{
    dynamic_fitter_factory.reset( 
        new TableEntryMaker<Eigen::Dynamic,Eigen::Dynamic>() );
}

template void GaussFitter<true,true,true>::create_specializations<0>();
template void GaussFitter<true,false,true>::create_specializations<0>();
template void GaussFitter<false,true,true>::create_specializations<0>();
template void GaussFitter<false,false,true>::create_specializations<0>();

template class GaussFitter<true,true,true>;
template class GaussFitter<false,true,true>;
template class GaussFitter<true,false,true>;
template class GaussFitter<false,false,true>;

}
