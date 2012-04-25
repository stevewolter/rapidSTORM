#ifndef DSTORM_GUF_DATAEXTRACTOR_H
#define DSTORM_GUF_DATAEXTRACTOR_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <dStorm/engine/Image_decl.h>
#include "Spot.h"
#include "Optics.h"

namespace dStorm {
namespace fit_window {

class FittingRegion;

/** Interface for converting the contents of a ROI in an image to
 *  fitable data. */
class FittingRegionCreator {
public:
    typedef engine::Image2D Image;
    virtual ~FittingRegionCreator() {}
    std::auto_ptr<FittingRegion> extract_data( const Image& image, const Spot& position ) const
        { return extract_data_(image,position); }
private:
    virtual std::auto_ptr<FittingRegion> 
        extract_data_( const Image& image, const Spot& position ) const = 0;
};

class FittingRegionCreatorTable {
    const Optics& optics;
    boost::ptr_vector<FittingRegionCreator> table_;
    template <typename EvaluationSchedule>
    struct instantiator;
public:
    template <typename EvaluationSchedule>
    FittingRegionCreatorTable( EvaluationSchedule, const Optics& optics );
    FittingRegionCreatorTable( const Optics& optics );
    const FittingRegionCreator& get( int index ) const
        { return table_[index]; }
};

}
}

#endif
