#ifndef LOCPREC_RIPLEYK_H
#define LOCPREC_RIPLEYK_H

#include <dStorm/output/OutputSource.h>

namespace ripley_k {

class Output;

}

namespace dStorm {
namespace output {

template <>
std::auto_ptr<OutputSource> make_output_source<ripley_k::Output>();

}
}

#endif
