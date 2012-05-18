#include "debug.h"
#include "BackgroundDeviationEstimator.h"
#include <boost/mpl/vector.hpp>
#include <boost/shared_array.hpp>
#include <dStorm/engine/Image.h>
#include <dStorm/image/iterator.h>
#include <dStorm/image/slice.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/input/Source.h>
#include <simparm/Entry.hh>
#include <simparm/Message.hh>
#include <simparm/Object.hh>
#include <simparm/Structure.hh>

using namespace dStorm::engine;

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
: public input::AdapterSource<engine::ImageStack>,
  boost::noncopyable
{
    int confidence_limit, binning;

  public:
    Source(std::auto_ptr< input::Source<engine::ImageStack> > base);
    TraitsPtr get_traits( Wishes );
};
class ChainLink
: public input::Method<ChainLink>
{
    friend class input::Method<ChainLink>;
    typedef boost::mpl::vector< dStorm::engine::ImageStack > SupportedTypes;
    bool ignore_unknown_type() const { return true; }
    template <typename Type>
    bool changes_traits( const input::MetaInfo&, const input::Traits<Type>& )
        { return false; }
    template <typename Type>
    input::Source<Type>* make_source( std::auto_ptr< input::Source<Type> > p ) {
        if ( config.enable() )
            return new BackgroundStddevEstimator::Source(p);
        else
            return p.release();
    }

    simparm::Structure<Config>& get_config() { return config; }
    simparm::Structure<Config> config;
  public:
    void attach_ui( simparm::Node& at ) { config.attach_ui( at ); }
    static std::string getName() { return "BackgroundEstimator"; }
};

Config::Config() 
: simparm::Object( ChainLink::getName(), "Estimate background variance"),
  enable("Enable", "Estimate background variance", true)
{
    enable.userLevel = simparm::Object::Intermediate;
}

ImagePlane::ImagePlane( int binning ) 
: binning(binning), highest_bin(1), total_counts(0)
{
    int hist_size = (int(std::numeric_limits<engine::StormPixel>::max())+1) >> binning;
    histogram.reset( new int[hist_size]);
    for (int i = 0; i < hist_size; i++) histogram[i] = 0;

}

void ImagePlane::add_image( const Image<engine::StormPixel,2>& im )
{
    for ( Image<engine::StormPixel,2>::const_iterator i = im.begin(), e = im.end(); i != e; ++i ) {
        if ( ++histogram[ *i >> binning ] > histogram[highest_bin] )
            highest_bin = *i >> binning;
    }
    total_counts += im.size_in_pixels();
}

bool ImagePlane::converged() const {
    return ( total_counts >= 100000 );
}

camera_response ImagePlane::background_stddev() const
{
    if ( highest_bin == 0 ) {
        throw lowest_histogram_mode_is_strongest();
    } else {
        double S = 0, sumweight = 0;
        double mean = -1, variance = -1;

        mean = variance = 0;
        for (int i = 0; i <= highest_bin; ++i) {
            if ( histogram[i] == 0 ) continue;
            double newweight = sumweight + histogram[i];
            double Q = ((i << binning) - mean), R = Q * histogram[i] / newweight;
            S += sumweight * Q * R;
            mean += R;
            sumweight = newweight;
            DEBUG("Mean changed to " << mean << " and variance to " << S * (highest_bin+1) / ( (highest_bin) * sumweight) );
        }
        variance = S * (highest_bin+1) / ( (highest_bin) * sumweight);
        return float(sqrt(variance)) * camera::ad_count;
    }

}

Source::Source(std::auto_ptr<input::Source<engine::ImageStack> > base)
: input::AdapterSource<engine::ImageStack>(base),
  confidence_limit(8), binning(3)
{
}

Source::TraitsPtr 
Source::get_traits( Wishes w )
{
    if ( ! w.test( InputStandardDeviation ) ) return base().get_traits(w);

    w.set( Repeatable );
    DEBUG("Running background standard deviation estimation");
    Source::TraitsPtr s = base().get_traits(w);
    if ( ! base().capabilities().test( Repeatable ) ) return s;
    std::vector<ImagePlane> planes( s->plane_count(), ImagePlane(binning) );

    for (input::Source<engine::ImageStack>::iterator i = base().begin(), e = base().end(); i != e; ++i ) {
        DEBUG("Loaded image " << i->frame_number());
        DEBUG("Adding image to histogram");
        
        bool all_images_converged = true;
        for (int j = 0; j < s->plane_count(); ++j) {
            if ( i->plane(j).is_invalid() )
                continue;
            planes[j].add_image( i->plane(j) );
            all_images_converged = all_images_converged && planes[j].converged();
        }
        if ( all_images_converged ) break;
    }

    for (int j = 0; j < s->plane_count(); ++j) 
        try {
            s->plane(j).optics.background_stddev = planes[j].background_stddev();
        } catch ( const lowest_histogram_mode_is_strongest& ) {
            simparm::Message m("Background standard deviation estimation failed",
                "The most common pixel in this image is also the lowest. I cannot compute a background standard deviation, sorry. "
                "Some things, like determining precision of localization, will not work.",
                simparm::Message::Warning);
            base().getNode().send(m);
        }

    DEBUG("Dispatching restart message");
    base().dispatch( BaseSource::RepeatInput );
    DEBUG("Finished background standard deviation estimation");
    
    return s;
}

std::auto_ptr<input::Link> makeLink() {
    return std::auto_ptr<input::Link>( new ChainLink() );
}

}
}
