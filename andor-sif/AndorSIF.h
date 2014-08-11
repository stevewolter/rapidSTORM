#ifndef CImgBuffer_ANDORSIF_H
#define CImgBuffer_ANDORSIF_H

#include <memory>
#include "engine/Image.h"
#include "input/Link.h"

namespace dStorm {
namespace andor_sif {

std::unique_ptr< input::Link<engine::ImageStack> > make_input();

}
}

#endif  /* double inclusion prevention */
