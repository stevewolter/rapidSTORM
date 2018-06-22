#ifndef DSTORM_FITWINDOW_CHUNKIFY_H
#define DSTORM_FITWINDOW_CHUNKIFY_H

#include "fit_window/Plane.h"
#include "nonlinfit/plane/DisjointData.h"
#include "nonlinfit/plane/JointData.h"

namespace dStorm {
namespace fit_window {

template <typename Number, int ChunkSize>
inline void chunkify_data_chunks(
        const fit_window::Plane& input,
        std::vector<nonlinfit::DataChunk<Number, ChunkSize>>& output);

template <typename Number, int ChunkSize>
void chunkify(const fit_window::Plane& input, nonlinfit::plane::DisjointData<Number, ChunkSize>& output);

template <typename Number, int ChunkSize>
void chunkify(const fit_window::Plane& input, nonlinfit::plane::JointData<Number, ChunkSize>& output);

}
}

#endif
