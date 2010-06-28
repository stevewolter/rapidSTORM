#ifndef FITPP_EXPONENTIAL_COMMON_DECL_H
#define FITPP_EXPONENTIAL_COMMON_DECL_H

namespace fitpp {
namespace Exponential {

static const int 
        Shift = 0, /**< B. Only one for all functions */
        MeanX = 1, /**< x0 */
        MeanY = 2, /**< y0 */
        Amplitude = 3; /**< A */

template <int Lines, int Size, bool ComputeExp>
struct PrecalculatedLines;
template <typename Space, int Width, int Height, bool Corr>
struct ParameterHelper;
template <typename Super, bool Corr>
struct DerivativeHelper;

}
}

#endif
