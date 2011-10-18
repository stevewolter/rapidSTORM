#ifndef DSTORM_LOCALIZEDIMAGE_ITERATOR_H
#define DSTORM_LOCALIZEDIMAGE_ITERATOR_H

#include "LocalizedImage.h"
#include "../localization/iterator.h"

namespace dStorm {
namespace output {

Localization::iterator LocalizedImage::begin() { return Localization::iterator(first); }
Localization::iterator LocalizedImage::end() { return Localization::iterator(NULL); }
Localization::const_iterator LocalizedImage::begin() const { return Localization::iterator(first); }
Localization::const_iterator LocalizedImage::end() const { return Localization::iterator(NULL); }

}
}

#endif
