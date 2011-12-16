#include "debug.h"
#include "BackgroundDeviationEstimator.h"
#include <dStorm/input/Source_impl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/image/iterator.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Filter_impl.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/image/slice.h>
#include <simparm/Message.hh>

using namespace dStorm::engine;

namespace dStorm {

namespace input {
namespace chain {

template <>
template <typename Type>
bool DefaultVisitor<BackgroundStddevEstimator::Config>::operator()( input::Traits<Type>& t ) {
    return true;
}

template <>
bool DefaultVisitor<BackgroundStddevEstimator::Config>::operator()( Context& c ) {
    return true;
}

template <>
template <typename Type>
bool DefaultVisitor<BackgroundStddevEstimator::Config>::operator()( std::auto_ptr< input::Source<Type> > p ) {
    new_source.reset( new BackgroundStddevEstimator::Source(p) );
    return true;
}

}
}

namespace BackgroundStddevEstimator {

Config::Config() 
: simparm::Object("BackgroundEstimator", "Estimate background variance"),
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

Source::Source(std::auto_ptr<input::Source<engine::Image> > base)
: input::AdapterSource<engine::Image>(base),
  confidence_limit(8), binning(3)
{
}

Source::TraitsPtr 
Source::get_traits( Wishes w )
{
    if ( ! w.test( InputStandardDeviation ) ) return base().get_traits(w);

    w.set( MultiplePasses );
    DEBUG("Running background standard deviation estimation");
    Source::TraitsPtr s = base().get_traits(w);
    if ( ! base().capabilities().test( Repeatable ) ) return s;
    std::vector<ImagePlane> planes( s->plane_count(), ImagePlane(binning) );

    for (input::Source<engine::Image>::iterator i = base().begin(), e = base().end(); i != e; ++i ) {
        DEBUG("Loaded image " << i->frame_number());
        if ( i->is_invalid() ) continue;
        DEBUG("Got image of size " << i->width_in_pixels() << " " << i->height_in_pixels() << " " << i->depth_in_pixels());
        DEBUG("Adding image to histogram");
        
        bool all_images_converged = true;
        for (int j = 0; j < s->plane_count(); ++j) {
            planes[j].add_image( i->slice(2, j * camera::pixel) );
            all_images_converged = all_images_converged && planes[j].converged();
        }
        if ( all_images_converged ) break;
    }

    for (int j = 0; j < s->plane_count(); ++j) 
        try {
            s->plane(j).background_stddev = planes[j].background_stddev();
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

input::chain::Link::AtEnd
ChainLink::traits_changed( ChainLink::TraitsRef r, Link* l) {
    return input::chain::DelegateToVisitor::traits_changed(*this,r,l);
}

input::chain::Link::AtEnd
ChainLink::context_changed( ChainLink::ContextRef c, Link* l)
{
    return input::chain::DelegateToVisitor::context_changed(*this,c,l);
}

input::BaseSource* ChainLink::makeSource()
{
    if ( ! config.enable() )
        return Forwarder::makeSource();
    else
        return input::chain::DelegateToVisitor::makeSource(*this);
}

std::auto_ptr<input::chain::Filter> makeLink() {
    return std::auto_ptr<input::chain::Filter>( new ChainLink() );
}

}
}
