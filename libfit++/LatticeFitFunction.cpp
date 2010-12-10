#define LIBFITPP_LATTICEFITFUNCTION_CPP
#include "LatticeFitFunction.hh"

using namespace std;
#include <iostream>

namespace libfitpp {

template <typename DataType>
LatticeFitFunction<DataType>::
LatticeFitFunction()
    throw()
: data(NULL)
{
}

template <typename DataType>
LatticeFitFunction<DataType>::~LatticeFitFunction() throw() 
{
}

template class LatticeFitFunction<char>;
template class LatticeFitFunction<unsigned char>;
template class LatticeFitFunction<short>;
template class LatticeFitFunction<unsigned short>;
template class LatticeFitFunction<int>;
template class LatticeFitFunction<unsigned int>;
template class LatticeFitFunction<long>;
template class LatticeFitFunction<unsigned long>;
template class LatticeFitFunction<long long>;
template class LatticeFitFunction<unsigned long long>;
template class LatticeFitFunction<float>;
template class LatticeFitFunction<double>;
template class LatticeFitFunction<long double>;

}
