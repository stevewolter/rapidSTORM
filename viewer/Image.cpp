#include "viewer/Image.h"
#include "dStorm/image/Image.h"

namespace dStorm {

template <typename Type, int D>
const int Image<Type,D>::Dim;

template const int Image<int,3>::Dim;

}
