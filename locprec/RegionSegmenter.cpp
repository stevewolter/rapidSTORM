#include "RegionSegmenter.h"
#include "foreach.h"
#include <limits>
#include <stdio.h>
#include <dStorm/Image_iterator.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/helpers/dilation.h>
#include <dStorm/helpers/back_inserter.h>
#include <dStorm/engine/CandidateTree.h>
#include <boost/units/quantity.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/Eigen/Array>
#include <boost/ptr_container/ptr_map.hpp>
#include <dStorm/output/Localizations_iterator.h>
#include <boost/foreach.hpp>

#include <dStorm/Image_impl.h>
#include <dStorm/helpers/dilation_impl.h>
#include <dStorm/output/binning/binning.h>

namespace locprec {

using namespace std;
using namespace dStorm;
using namespace dStorm::output;
using namespace dStorm::engine;
using namespace dStorm::outputs;

Segmenter::_Config::_Config()
:   simparm::Object("Segmenter", "Segment target image"),
    method("SegmentationMethod", "Method for segmentation"),
    threshold("SegmentationThreshold","Threshold for regionness", 0.001),
    dilation("SegmentationDilation", 
            "Region dilation in binned pixels", 0),
    save_segmentation("SaveSegmentation", "Save segmentation"),
    load_segmentation("LoadSegmentation", "Load segmentation")
{
    method.addChoice( Segmenter::Maximum,       
        "Maximum", "Local maximums");
    method.addChoice( Segmenter::Region, 
        "Regions", "Coherent regions");
    method = Segmenter::Maximum;

    trace_filter.show_in_tree = false;
}

void Segmenter::_Config::registerNamedEntries()
{
    method["Maximum"].push_back( selector );

    method["Regions"].push_back( threshold );
    method["Regions"].push_back( dilation );
    method["Regions"].push_back( selector );
    method["Regions"].push_back( load_segmentation );
    method["Regions"].push_back( save_segmentation );

    push_back( method );
    push_back( reducer );
    push_back( trace_filter );
}


Segmenter::Segmenter(
    const Config& config,
    std::auto_ptr<Output> output
)
: Crankshaft("Segmenter"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  howToSegment( config.method() ),
  threshold( config.threshold ),
  dilation( config.dilation ),
  output( new TraceCountFilter( config.trace_filter, output ) ),
  reducer( config.reducer.make_trace_reducer() ),
  load_segmentation( config.load_segmentation() ),
  save_segmentation( config.save_segmentation() ) 
{
    desc = "Region segmenter";

    bins = new dStorm::outputs::BinnedLocalizations<dStorm::outputs::DummyBinningListener>(config.selector.make());
    binners.replace(0, config.selector.make_x());
    binners.replace(1, config.selector.make_y());

    add( new dStorm::outputs::LocalizationList(&points) );
    add( bins );

    if ( howToSegment == Region ) {
        receive_changes_from( this->threshold.value );
        push_back( this->threshold );
        receive_changes_from( this->dilation.value );
        push_back( this->dilation );
    }

    push_back( this->output->getNode() );
}

Segmenter::~Segmenter() {
}

Output::AdditionalData Segmenter::announceStormSize
            (const Announcement &a) 
{
    ost::MutexLock lock( mutex );
    announcement.reset( new Announcement(a) );
    typedef dStorm::input::Traits<dStorm::Localization> InputTraits;
    announcement->source_traits.push_back( 
        boost::shared_ptr<InputTraits>( new InputTraits(a) ) );
    /* TODO: Evaluate return */
    output->announceStormSize(*announcement);
    return Crankshaft::announceStormSize(a);
}

void Segmenter::operator()(const simparm::Event&)
{
    ost::MutexLock lock(mutex);
    segment();
}

static dStorm::Pixel colors[] = {
    dStorm::Pixel::Red(),
    dStorm::Pixel::Green(),
    dStorm::Pixel::Blue(),
    dStorm::Pixel(0, 255, 0),
    dStorm::Pixel(0, 0, 255),
    dStorm::Pixel(0, 255, 255),
    dStorm::Pixel(255, 0, 255),
    dStorm::Pixel(255, 255, 0),
    dStorm::Pixel(127, 0, 0),
    dStorm::Pixel(0, 127, 0),
    dStorm::Pixel(0, 0, 127),
    dStorm::Pixel(0, 127, 127),
    dStorm::Pixel(127, 0, 127),
    dStorm::Pixel(127, 127, 0),
};

struct Transformer {
    dStorm::Pixel operator()( int i ) {
        if ( i == 0 )
            return 0;
        else {
            return colors[i % (sizeof(colors)/sizeof(colors[0]))];
        }
    }
};

Segmenter::ColorImage
Segmenter::color_regions(
    const RegionImage& regImage
) {
    ColorImage coloredImage( regImage.sizes() );
    std::transform( regImage.begin(), regImage.end(), 
                    coloredImage.begin(), Transformer() );
    return coloredImage;
}

static void merge(std::vector<int>& labArray, int a, int b) {
    int region = std::min(labArray[a], labArray[b]);
    int merged = std::max(labArray[a], labArray[b]);

    std::replace( labArray.begin(), labArray.end(), merged, region );
}

Segmenter::RegionImage Segmenter::segment_image()
{
    const dStorm::outputs::BinnedImage& src = (*bins)();
    dStorm::Image<bool,2> thres = src.threshold( threshold() ),
               dilated( src.sizes() );
    if ( dilation() > 0 )
        dStorm::rectangular_dilation( thres, dilated, 
                                      dilation(), dilation(), 0, 0 );
    else
        dilated = thres;

    RegionImage labImage(src.sizes());
    int curLab = 0;
    const int Background = 0, NoRegion = -1;
    std::vector<int> labArray;
    labArray.push_back( Background );

    /* X/Y offset pairs for a forward scan. */
    const int forward_scan[][2] = { {-1,-1}, {0,-1}, {1,-1}, {-1,0} };
    const int forward_scan_offsets = 4;

    for (int y = 0; y < int(src.height_in_pixels()); y++)
      for (int x = 0; x < int(src.width_in_pixels()); x++) {
        if ( dilated(x,y) == false ) {
            labImage(x,y) = Background;
            continue;
        }

        labImage(x,y) = NoRegion;
        for (int n = 0; n < forward_scan_offsets; n++) {
            int dx = forward_scan[n][0]+x, dy = forward_scan[n][1]+y;
            if ( dx < 0 || dy < 0 || dx > int(labImage.width_in_pixels()) )
                continue;

            if ( dilated(x,y) == dilated(dx,dy) ) {
                if ( labImage(x,y) == NoRegion ) {
                    labImage(x,y) = labImage(dx,dy);
                } else if ( labImage(x,y) != labImage(dx,dy) ) {
                    merge( labArray, labImage(x,y), labImage(dx,dy) );
                }
            }
        }

        if (labImage(x,y) == NoRegion) {
            curLab++;
            labArray.push_back(curLab);
            labImage(x,y) = curLab;
        }
    }

    int cLAi = 0;
    std::vector<int> compressedLabArray(labArray.size(), -1);
    for (int i = 0; i < int(labArray.size()); i++) {
        if ( i == labArray[i] )
            compressedLabArray[i] = cLAi++;
        else
            compressedLabArray[i] = compressedLabArray[labArray[i]];
    }

    for (RegionImage::iterator i = labImage.begin(); i != labImage.end(); i++)
        *i = compressedLabArray[*i];

    display_image( color_regions(labImage) );

    return labImage;
}

void Segmenter::segment()
{
    RegionImage segmentation;
    if ( load_segmentation != "" ) {
        FILE *file = fopen( load_segmentation.c_str(), "r" );
        segmentation = RegionImage( (*bins)().sizes() );
        int read = fread( segmentation.ptr(), sizeof(int),
               segmentation.size_in_pixels(), file );
        if ( read != int(segmentation.size_in_pixels()) )
            throw std::runtime_error("Invalid segmentation file given");
        fclose(file);
    } else 
        segmentation = segment_image();

    if ( save_segmentation != "" ) {
        FILE *file = fopen( load_segmentation.c_str(), "w" );
        fwrite( segmentation.ptr(), sizeof(int),
                segmentation.size_in_pixels(), file );
        fclose(file);
    }

    typedef std::vector<Localization> Trace;
    std::vector<Trace*> srMap(segmentation.minmax().second+1,
                                      (Trace*)NULL);
    std::list<Trace> regions;

    for ( Localizations::const_iterator fit = points.begin(); 
             fit != points.end(); fit++)
    {
        int x_bin = round(binners[0].bin_point(*fit)),
            y_bin = round(binners[0].bin_point(*fit));
        int region = segmentation(x_bin, y_bin);
        if (srMap[region] == NULL) {
            regions.push_back( Trace() );
            regions.back().push_back( *fit );
            srMap[region] = &regions.back();
        } else {
            Trace &sr = *srMap[region];
            sr.push_back(*fit);
        }
    }

    std::list<Trace>::iterator i;
    EngineResult engineResult;
    engineResult.forImage = frame_count::from_value(-1);
    for ( i = regions.begin(); i != regions.end(); i++) {
        reducer->reduce_trace_to_localization( i->begin(), i->end(), boost::back_inserter(engineResult),
            dStorm::samplepos::Constant( 0 * si::meter ) );
    }

    output->propagate_signal(Engine_is_restarted);
    output->receiveLocalizations(engineResult);
    output->propagate_signal(Engine_run_succeeded);
}

template <typename To> To sq(To a) { return a*a; }

template <typename From, typename To>
class LocalizationMapper 
    : public unary_function<void, From>
{
    const std::list<To>& spots;
    const boost::ptr_array< dStorm::output::binning::Unscaled, 2 >& binners;
    typedef std::vector<From> Store;
  public:
    typedef boost::ptr_map<const To*, Store > Map;

    LocalizationMapper(const std::list<To>& spots, const boost::ptr_array< dStorm::output::binning::Unscaled, 2 >& binners)
        : spots(spots), binners(binners)
    {
    }

    void operator()(const From& loc)
    {
        double minDist = std::numeric_limits<double>::max();
        const To* minCand = NULL;
        float locpos[2];
        for (int i = 0; i < 2; ++i)
            locpos[i] = binners[i].bin_point(loc);
        BOOST_FOREACH( const Spot& spot, spots )
        {
            double dist = sq(spot.x() - locpos[0]) +
                            sq(spot.y() - locpos[1]);
            if (dist < minDist) {
                minDist = dist;
                minCand = &spot;
            }
        }
        assert( minCand );
        mapping[minCand].push_back( loc );
    }

    const Map& getMapping() const { return mapping; }

  private:
    Map mapping;
};

typedef 
LocalizationMapper<Localization, Spot> Mapper;

void Segmenter::maximums() {
    dStorm::engine::CandidateTree<float> candidates(3,3,0,0);
    candidates.setLimit(100000);
    const dStorm::outputs::BinnedImage& image = (*bins)();
    candidates.fillMax(image);
    std::list<Spot> foundSpots;
    for (dStorm::engine::CandidateTree<float>::iterator
            it = candidates.begin(); it.hasMore(); it++)
        foundSpots.push_back( Spot( it->second.x(), it->second.y() ) );

    Localizations& locs = points;
    Mapper mapper(foundSpots, binners);
    for (Localizations::const_iterator i= locs.begin(); 
                                               i!= locs.end(); i++)
        mapper(*i);

    EngineResult engineResult;
    engineResult.forImage = frame_count::from_value(0);

    for ( Mapper::Map::const_iterator i = mapper.getMapping().begin();
          i != mapper.getMapping().end(); i++)
    {
        reducer->reduce_trace_to_localization( 
            i->second->begin(), i->second->end(),
            boost::back_inserter( engineResult ),
            dStorm::samplepos::Constant( 0 * si::meter )
        );
    }

    output->propagate_signal(Engine_is_restarted);
    output->receiveLocalizations(engineResult);
    output->propagate_signal(Engine_run_succeeded);
}

void Segmenter::display_image( const ColorImage& img ) {
    if ( announcement.get() == NULL ) return;
    dStorm::Display::ResizeChange new_size;
    new_size.size = img.sizes();
    new_size.keys.push_back( dStorm::Display::KeyDeclaration( "Segment", "Index of associated region", 1+(sizeof(colors)/sizeof(colors[0])) ));
    for (int i = 0; i < 2; ++i)
        new_size.pixel_sizes[i] = binners[i].resolution();

    if ( display.get() == NULL ) {
        dStorm::Display::Manager::WindowProperties props;
        props.initial_size = new_size;

        display = dStorm::Display::Manager::getSingleton()
            .register_data_source( props, *this );
        next_change.reset( new dStorm::Display::Change(1) );
        next_change->changed_keys.front().reserve( new_size.keys.front().size );
        for (int i = 0; i < new_size.keys.front().size; i++)
            next_change->changed_keys.front().push_back( dStorm::Display::KeyChange(
                i, (i == 0) ? dStorm::Pixel::Black() : colors[i-1], i ) );
    } else {
        next_change->do_resize = true;
        next_change->resize_image = new_size;
    }

    next_change->do_change_image = true;
    next_change->image_change.new_image = img;
}

std::auto_ptr<dStorm::Display::Change> Segmenter::get_changes() {
    std::auto_ptr<dStorm::Display::Change> fresh( new dStorm::Display::Change(1) );
    ost::MutexLock lock(mutex);
    std::swap( fresh, next_change );
    return fresh;
}

}

namespace dStorm {
//template class Image<int,2>;
//template class Image<bool,2>;
template void rectangular_erosion<unsigned int, unsigned int>(dStorm::Image<unsigned int, 2> const&, dStorm::Image<unsigned int, 2>&, int, int, int, int);
template void rectangular_dilation<unsigned int, unsigned int>(dStorm::Image<unsigned int, 2> const&, dStorm::Image<unsigned int, 2>&, int, int, int, int);
}
