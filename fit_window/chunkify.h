#ifndef DSTORM_FITWINDOW_CHUNKIFY_H
#define DSTORM_FITWINDOW_CHUNKIFY_H

#include "fit_window/Plane.h"
#include "nonlinfit/plane/DisjointData.h"
#include "nonlinfit/plane/JointData.h"

namespace dStorm {
namespace fit_window {

template <bool NeedLogOutput, typename Number, int ChunkSize>
void chunkify(const fit_window::Plane& input, nonlinfit::plane::DisjointData<Number, ChunkSize>& output);

template <bool NeedLogOutput, typename Number, int ChunkSize>
void chunkify(const fit_window::Plane& input, nonlinfit::plane::JointData<Number, ChunkSize>& output);

}
}

#endif
