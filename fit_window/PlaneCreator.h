#ifndef DSTORM_GUF_DATAEXTRACTOR_H
#define DSTORM_GUF_DATAEXTRACTOR_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <dStorm/engine/Image_decl.h>
#include "Spot.h"
#include "Optics.h"

namespace dStorm {
namespace fit_window {

class Plane;

/** Interface for converting the contents of a ROI in an image to
 *  fitable data. */
class PlaneCreator {
public:
    typedef engine::Image2D Image;
    virtual ~PlaneCreator() {}
    std::auto_ptr<Plane> extract_data( const Image& image, const Spot& position ) const
        { return extract_data_(image,position); }
private:
    virtual std::auto_ptr<Plane> 
        extract_data_( const Image& image, const Spot& position ) const = 0;
};

class PlaneCreatorTable {
    const Optics& optics;
    boost::ptr_vector<PlaneCreator> table_;
    template <typename EvaluationSchedule>
    struct instantiator;
public:
    template <typename EvaluationSchedule>
    PlaneCreatorTable( EvaluationSchedule, const Optics& optics );
    const PlaneCreator& get( int index ) const
        { return table_[index]; }
};

}
}

#endif
