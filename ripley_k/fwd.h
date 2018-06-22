#ifndef LOCPREC_RIPLEYK_H
#define LOCPREC_RIPLEYK_H

#include "output/OutputSource.h"

namespace dStorm {
namespace ripley_k {

std::auto_ptr<dStorm::output::OutputSource> make_output_source();

}
}

#endif
