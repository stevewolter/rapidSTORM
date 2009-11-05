#include "Source.h"

namespace CImgBuffer {

BaseSource::BaseSource(BaseSource::Flags flags)
    : _pushes(flags & Pushing), _canBePulled(flags & Pullable),
      _managed(flags & Managing), 
      _concurrent(flags & Concurrent),
      roi_start(0), roi_end(std::numeric_limits<unsigned int>::max())
    {}


BaseSource::~BaseSource() {}

unsigned int BaseSource::number_of_objects() const { 
    unsigned int li = std::min(roi_end,(unsigned int)quantity()-1);
    return li - roi_start + 1;
}

}
