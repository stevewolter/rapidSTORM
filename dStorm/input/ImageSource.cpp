#include <dStorm/input/Source_impl.h>
#include <dStorm/Image_decl.h>

namespace dStorm {
namespace input {
template class Source< dStorm::Image<unsigned int,2> >;
template class Source< dStorm::Image<unsigned short,2> >;
template class Source< dStorm::Image<unsigned char,2> >;
template class Source< dStorm::Image<float,2> >;
template class Source< dStorm::Image<double,2> >;
}
}
