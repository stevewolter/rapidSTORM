#ifndef DSTORM_GUF_INPUTPLANE_H
#define DSTORM_GUF_INPUTPLANE_H

#include "Spot.h"
#include "Config.h"
#include <dStorm/engine/Image_decl.h>

#include "Optics.h"
#include "ScheduleIndexFinder.h"
#include "DataExtractor.h"

namespace dStorm {
namespace guf {

template <int Dim> class Statistics;
class DataPlane;

class InputPlane;

class InputPlane {
public:
    typedef engine::Image2D Image;
private:
    Optics optics;
    ScheduleIndexFinder index_finder;
    DataExtractorTable extractor_table;

public:
    InputPlane( const Config&, const engine::InputPlane& );
    std::auto_ptr<DataPlane> set_image( const Image& image, const Spot& position ) const;
};

}
}

#endif
