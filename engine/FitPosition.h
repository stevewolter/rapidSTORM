#ifndef DSTORM_ENGINE_FITPOSITION_H
#define DSTORM_ENGINE_FITPOSITION_H

#include <Eigen/Core>

namespace dStorm {
namespace engine {

// Promising fit start position in X and Y in micrometers.
typedef Eigen::Matrix< double, 2, 1, Eigen::DontAlign > FitPosition;

}
}

#endif
