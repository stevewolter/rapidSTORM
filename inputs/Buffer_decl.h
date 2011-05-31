/** \file Buffer_decl.h
 *  This file contains the declaration for the Buffer class.
 */
#ifndef DSTORM_BUFFER_DECL_H
#define DSTORM_BUFFER_DECL_H

#include <dStorm/input/chain/Filter_decl.h>

namespace dStorm {
namespace input {
    template <typename Ty> class Buffer;
    class BufferChainLink;
    std::auto_ptr<chain::Filter> makeBufferChainLink();
}
}

#endif
