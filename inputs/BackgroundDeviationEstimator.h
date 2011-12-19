#ifndef DSTORM_INPUT_BACKGROUND_STDDEV_ESTIMATOR_H
#define DSTORM_INPUT_BACKGROUND_STDDEV_ESTIMATOR_H

#include "BackgroundDeviationEstimator_decl.h"
#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/engine/Image.h>
#include <dStorm/input/Method.hpp>
#include <simparm/Entry.hh>
#include <boost/mpl/vector.hpp>
#include <boost/shared_array.hpp>

namespace dStorm {
namespace BackgroundStddevEstimator {

struct Config : public simparm::Object
{
    simparm::BoolEntry enable;
    Config();
    void registerNamedEntries() { push_back(enable); }
};

struct lowest_histogram_mode_is_strongest : public std::runtime_error {
    lowest_histogram_mode_is_strongest() 
        : std::runtime_error("The lowest histogram pixel in the background estimation is the strongest") {}
};

struct ImagePlane {
    ImagePlane( int binning );
    void add_image( const Image<engine::StormPixel,2>& );
    bool converged() const;
    camera_response background_stddev() const;
  private:
    const int binning;
    int highest_bin, total_counts;
    boost::shared_array<int> histogram;
};

class Source 
: public input::AdapterSource<engine::Image>,
  boost::noncopyable
{
    int confidence_limit, binning;

  public:
    Source(std::auto_ptr< input::Source<engine::Image> > base);
    TraitsPtr get_traits( Wishes );
};

}
}

#endif
