#include "debug.h"
#include "BackgroundDeviationEstimator.h"
#include <dStorm/input/Source_impl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/image/iterator.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context.h>
#include <simparm/Message.hh>

using namespace dStorm::engine;

namespace dStorm {
namespace BackgroundStddevEstimator {

Config::Config() 
: simparm::Object("BackgroundEstimator", "Estimate background variance"),
  disable("Disable", "Disable background variance estimation")
{
}

static void add_to_histogram( const engine::Image& im, int h[], const int b ) 
{
    for ( engine::Image::const_iterator i = im.begin(), e = im.end(); i != e; ++i ) 
    {
        h[ *i >> b ] += 1;
    }
}

Source::Source(std::auto_ptr<input::Source<engine::Image> > base)
: input::Source<engine::Image>(base->getNode(), base->flags),
  base(base), confidence_limit(8), binning(3)
{
}

Source::TraitsPtr 
Source::get_traits()
{
    Source::TraitsPtr s = base->get_traits();
    int hist_size = (int(std::numeric_limits<engine::StormPixel>::max())+1) >> binning;
    int histogram[hist_size];
    for (int i = 0; i < hist_size; i++) histogram[i] = 0;

    int highest_bin = 1, histogram_size = 0;
    double mean = -1, variance = -1;
    for (input::Source<engine::Image>::iterator i = base->begin(), e = base->end(); i != e; ++i ) {
        DEBUG("Loaded image " << i->frame_number());
        if ( i->is_invalid() ) continue;
        DEBUG("Got image of size " << i->width_in_pixels() << " " << i->height_in_pixels());
        DEBUG("Adding image to histogram");
        add_to_histogram( *i, histogram, binning );
        
        highest_bin = 0;
        for (int j = 1; j < hist_size; ++j)  {
            //DEBUG("Image " << i->frame_number().value() << " bin " << (j << binning) << " " << histogram[j]);
            if ( histogram[highest_bin] < histogram[j] )
                highest_bin = j;
        }
        histogram_size = 0;
        for (int i = 0; i <= highest_bin; ++i) histogram_size += histogram[i];
        DEBUG("Histogram has " << histogram_size << " pixels and highest bin is " << highest_bin);
        if ( histogram_size >= 100000 ) break;
    }

    if ( highest_bin == 0 ) {
        simparm::Message m("Background standard deviation estimation failed",
            "The most common pixel in this image is also the lowest. I cannot compute a background standard deviation, sorry. "
            "Some things, like determining precision of localization, will not work",
            simparm::Message::Warning);
        base->getNode().send(m);
    } else if ( histogram_size < 100000 ) {
        simparm::Message m("Background standard deviation estimation failed",
            "Too few pixels in input data to estimate background standard deviation. At least 100,000 pixels are necessary.",
            simparm::Message::Warning);
        base->getNode().send(m);
    } else {
        double S = 0, sumweight = 0;
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
        s->background_stddev = float(sqrt(variance)) * cs_units::camera::ad_count;
    }

    base->dispatch( BaseSource::RepeatInput );
    
    return s;
}

input::chain::Link::AtEnd
ChainLink::traits_changed(
    ChainLink::TraitsRef r,
    Link*,
    boost::shared_ptr< input::Traits<engine::Image> > t
) {
    if ( r.get() == NULL )
        return this->notify_of_trait_change(r);
    else {
        input::chain::MetaInfo::Ptr mr( r->clone() );
        boost::shared_ptr< input::Traits<engine::Image> > tc(t->clone());
        mr->traits = tc;
        tc->background_stddev.promise( deferred::JobTraits );
        return notify_of_trait_change(mr);
    }
}

input::chain::Link::AtEnd
ChainLink::context_changed(
    ChainLink::ContextRef c,
    Link* l)
{
    Link::context_changed(c,l);
    if ( c.get() != NULL ) {
        input::chain::Context::Ptr mc( c->clone() );
        mc->will_make_multiple_passes = true;
        return dispatch_context_change(mc, l, PassThrough);
    } else {
        return dispatch_context_change(c, l, PassThrough);
    }
}

input::BaseSource* ChainLink::makeSource( SourcePtr p )
{
    if ( config.disable() )
        return p.release();
    else
        return new Source(p);
}

std::auto_ptr<input::chain::Filter> makeLink() {
    return std::auto_ptr<input::chain::Filter>( new ChainLink() );
}

}
}
