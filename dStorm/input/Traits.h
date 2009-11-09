#ifndef DSTORM_INPUT_TRAITS_H
#define DSTORM_INPUT_TRAITS_H

namespace dStorm {
namespace input {
/** The Traits class confers information about objects in a Source.
 *  It can be specialized for concrete objects; in that case, make
 *  sure all users of the Source class see the relevant specialization. */
template <typename Type> class Traits;
}
}

#endif
