#ifndef CImgBuffer_ANDORSIF_H
#define CImgBuffer_ANDORSIF_H

#include <memory>
#include "input/Link.h"

namespace dStorm {
namespace andor_sif {

std::unique_ptr< input::Link > make_input();

}
}

#endif  /* double inclusion prevention */
