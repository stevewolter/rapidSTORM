#ifndef DSTORM_GUF_INPUTPLANE_H
#define DSTORM_GUF_INPUTPLANE_H

#include <Eigen/StdVector>
#include "TransformedImage.h"
#include "Spot.h"
#include "Config.h"
#include <boost/optional/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "guf/psf/LengthUnit.h"
#include <dStorm/engine/JobInfo.h>
#include <dStorm/traits/Projection.h>

namespace dStorm {
namespace guf {

template <int Dim> class Statistics;
class DataPlane;

class InputPlane;

class ScheduleIndexFinder {
    const bool do_disjoint, use_floats;
    const engine::InputPlane& plane;
    const InputPlane& input_plane;

    class set_if_appropriate;

public:
    ScheduleIndexFinder( const Config& config, const engine::InputPlane& plane, const InputPlane& input_plane );
    int get_evaluation_tag_index( const guf::Spot& position ) const;
};

class DataExtractor {
public:
    typedef engine::Image2D Image;
    virtual ~DataExtractor() {}
    std::auto_ptr<DataPlane> extract_data( const Image& image, const Spot& position ) const
        { return extract_data_(image,position); }
private:
    virtual std::auto_ptr<DataPlane> 
        extract_data_( const Image& image, const Spot& position ) const = 0;
};

class DataExtractorTable {
    const InputPlane& input;
    boost::ptr_vector<DataExtractor> table_;
    struct instantiator;
public:
    template <typename EvaluationSchedule>
    DataExtractorTable( EvaluationSchedule, const InputPlane& input );
    const DataExtractor& get( int index ) const
        { return table_[index]; }
};

class InputPlane {
public:
    typedef dStorm::engine::Image2D Image;
private:
    typedef guf::TransformedImage< PSF::LengthUnit > TransformedImage;

    ScheduleIndexFinder index_finder;
    DataExtractorTable extractor_table;

    traits::Projection::ImagePosition im_size;
    TransformedImage transformation;

    quantity< camera::intensity > photon_response_;
    quantity< camera::intensity, int > dark_current;
    boost::optional< float > background_noise_variance_;
    bool has_precision, poisson_background_;

    friend class DataPlane;
    friend class ScheduleIndexFinder;
    template <typename Tag> friend class TaggedDataPlane;

    int get_fit_window_width(const guf::Spot& at) const;
    template <typename Data>
    inline const Statistics<2> set_data(Data&, const Image&, const Spot&) const;

public:
    InputPlane( const Config&, const engine::InputPlane& );
    std::auto_ptr<DataPlane> set_image( const Image& image, const Spot& position ) const;
    bool can_compute_localization_precision() const { return has_precision; }
    float background_noise_variance() const { return *background_noise_variance_; }
    bool background_is_poisson_distributed() const 
        { return poisson_background_; }
    quantity< camera::intensity > photon_response() const { return photon_response_; }
};

}
}

#endif
