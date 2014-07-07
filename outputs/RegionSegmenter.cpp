#include "simparm/Eigen_decl.h"
#include "simparm/BoostUnits.h"
#include "simparm/Eigen.h"

#include <boost/thread/locks.hpp>
#include <boost/ptr_container/ptr_array.hpp>

#include "binning/config.h"
#include "output/OutputSource.h"
#include "density_map/DensityMap.h"
#include "density_map/CoordinatesFactory.h"
#include "density_map/DummyListener.h"
#include "engine/Image.h"
#include "display/Manager.h"
#include <boost/thread/mutex.hpp>
#include "simparm/Entry.h"
#include "simparm/ChoiceEntry.h"
#include "simparm/FileEntry.h"
#include <cassert>
#include "simparm/ObjectChoice.h"
#include "simparm/ManagedChoiceEntry.h"
#include "simparm/Node.h"

#include "outputs/RegionSegmenter.h"
#include <limits>
#include <stdio.h>
#include "image/iterator.h"
#include "engine/Spot.h"
#include "image/dilation.h"
#include "image/extend.h"
#include "helpers/back_inserter.h"
#include "engine/CandidateTree.h"
#include <boost/units/quantity.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/Eigen/Array>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/foreach.hpp>
#include "output/FilterBuilder.h"
#include "output/Filter.h"
#include "density_map/LinearInterpolation.h"

#include "image/Image.hpp"
#include "image/dilation_impl.h"
#include "binning/binning.h"

namespace dStorm { 
namespace outputs {

class Segmenter : public dStorm::output::Filter,
    private dStorm::display::DataSource
{
    public:
    class Config;
    enum SegmentationType { Maximum, Region };

    private:
    typedef dStorm::Image<dStorm::Pixel,2> ColorImage;
    typedef dStorm::Image<int,2> RegionImage;

    boost::mutex mutex;

    std::auto_ptr<Announcement> announcement;
    SegmentationType howToSegment;
    boost::ptr_array< binning::Unscaled, 2 > binners;
    simparm::Entry<double> threshold;
    simparm::Entry<unsigned long> dilation;
    std::vector<Localization> points;

    dStorm::density_map::DummyListener<2> dummy_binning_listener;
    dStorm::density_map::DensityMap< dStorm::density_map::DummyListener<2>, 2 > bins;

    std::auto_ptr< dStorm::display::Change > next_change;
    std::auto_ptr< dStorm::display::WindowHandle > display;

    std::auto_ptr< dStorm::output::Output > output;

    std::string load_segmentation, save_segmentation;
    simparm::BaseAttribute::ConnectionStore listening[2];
    simparm::NodeHandle user_interface;

    static ColorImage color_regions( const RegionImage& );
    void display_image( const ColorImage& );
    std::auto_ptr<dStorm::display::Change> get_changes();

    void store_results_( bool success ) {
        boost::lock_guard<boost::mutex> lock(mutex);
        if ( howToSegment == Maximum )
            maximums();
        else
            segment();
    }
    void attach_ui_( simparm::NodeHandle );

protected:
    RegionImage segment_image();
    void segment();
    void maximums();

public:
    Segmenter( const Config& config,
                std::auto_ptr<dStorm::output::Output> output);
    Segmenter( const Segmenter & );
    ~Segmenter();

    void announceStormSize(const Announcement &a) OVERRIDE;
    void receiveLocalizations(const EngineResult& er) OVERRIDE {
        std::copy(er.begin(), er.end(), back_inserter(points));
        bins.receiveLocalizations(er);
    }

    void segment_locked();
};

struct SegmentationMethod : public simparm::ObjectChoice {
    SegmentationMethod( std::string name, std::string desc )
        : simparm::ObjectChoice(name,desc) {}

    virtual ~SegmentationMethod() {}
    virtual void attach_ui( simparm::NodeHandle ) = 0;
    virtual SegmentationMethod* clone() const = 0;
    virtual Segmenter::SegmentationType type() const = 0;
};

struct MaximumSegmentationMethod : public SegmentationMethod {
    MaximumSegmentationMethod() : SegmentationMethod("Maximum", "Local maximums") {}
    virtual void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }
    virtual SegmentationMethod* clone() const { return new MaximumSegmentationMethod(*this); }
    virtual Segmenter::SegmentationType type() const { return Segmenter::Maximum; }
};

struct RegionSegmentationMethod : public SegmentationMethod {
    simparm::Entry<double> threshold;
    simparm::Entry<unsigned long> dilation;
    simparm::FileEntry save_segmentation, load_segmentation;

    RegionSegmentationMethod() 
    :   SegmentationMethod("Regions", "Coherent regions"),
        threshold("SegmentationThreshold","Threshold for regionness", 0.001),
        dilation("SegmentationDilation", 
                "Region dilation in binned pixels", 0),
        save_segmentation("SaveSegmentation", "Save segmentation", ""),
        load_segmentation("LoadSegmentation", "Load segmentation", "") {}
    void attach_ui( simparm::NodeHandle at ) { 
        simparm::NodeHandle r = attach_parent(at);
        threshold.attach_ui( r );
        dilation.attach_ui( r );
        load_segmentation.attach_ui( r );
        save_segmentation.attach_ui( r );
    }

    SegmentationMethod* clone() const { return new RegionSegmentationMethod(*this); }
    Segmenter::SegmentationType type() const { return Segmenter::Region; }
};

struct Segmenter::Config {
    simparm::ManagedChoiceEntry<SegmentationMethod> method;
    dStorm::density_map::CoordinatesFactory<2> selector;

    static std::string get_name() { return "Segmenter"; }
    static std::string get_description() { return "Segment target image"; }
    static simparm::UserLevel get_user_level() { return simparm::Intermediate; }

    Config();
    void attach_ui( simparm::NodeHandle at );
};

using namespace std;
using namespace dStorm;
using namespace dStorm::output;
using namespace dStorm::engine;

Segmenter::Config::Config()
:   method("SegmentationMethod", "Method for segmentation")
{
    method.addChoice( new MaximumSegmentationMethod() );
    method.addChoice( new RegionSegmentationMethod() );
    method.choose("Maximum");
}

void Segmenter::Config::attach_ui( simparm::NodeHandle at )
{
    selector.attach_ui( at );

    method.attach_ui( at );
}


Segmenter::Segmenter(
    const Config& config,
    std::auto_ptr<Output> output
)
: Filter(output),
  howToSegment( config.method().type() ),
  threshold( static_cast<const RegionSegmentationMethod&>( config.method["Regions"] ).threshold ),
  dilation( static_cast<const RegionSegmentationMethod&>( config.method["Regions"] ).dilation ),
  bins( &dummy_binning_listener, config.selector.make(), density_map::make_linear_interpolator<2>() ),
  load_segmentation( static_cast<const RegionSegmentationMethod&>( config.method["Regions"] ).load_segmentation() ),
  save_segmentation( static_cast<const RegionSegmentationMethod&>( config.method["Regions"] ).save_segmentation() ) 
{
    binners.replace(0, config.selector.make_x());
    binners.replace(1, config.selector.make_y());
}

Segmenter::~Segmenter() {
}

void Segmenter::announceStormSize(const Announcement &a) {
    boost::lock_guard<boost::mutex> lock( mutex );
    for (int i = 0; i < 2; ++i)
        binners[i].announce( a );
    announcement.reset( new Announcement(a) );
    announcement->group_field = input::GroupFieldSemantic::Molecule;
    announcement->input_image_traits.reset();
    bins.announceStormSize( a );
    Filter::announceStormSize( *announcement );
}

void Segmenter::segment_locked()
{
    boost::lock_guard<boost::mutex> lock(mutex);
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
    const dStorm::Image<float,2>& src = bins();
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
        segmentation = RegionImage( bins().sizes() );
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

    for ( const Localization& point : points ) {
        int bins[2];
        bool good = true;
        for (int i = 0; i < 2; ++i) {
            boost::optional<float> v = binners[i].bin_point(point);
            if ( ! v ) 
                good = false;
            else
                bins[i] = round(*v);
        }
        if ( ! good ) return;
        int region = segmentation(bins[0], bins[1]);
        if (srMap[region] == NULL) {
            regions.push_back( Trace() );
            regions.back().push_back( point );
            srMap[region] = &regions.back();
        } else {
            Trace &sr = *srMap[region];
            sr.push_back(point);
        }
    }

    std::list<Trace>::iterator i;
    EngineResult engineResult;
    Filter::announce_run(RunAnnouncement());
    int group = 0;
    for (const Trace& trace : regions) {
        EngineResult result;
        result.group = group++;
        std::copy(trace.begin(), trace.end(), std::back_inserter(result));
        Filter::receiveLocalizations(result);
    }
    Filter::store_children_results( true );
}

template <typename To> To sq(To a) { return a*a; }

template <typename From, typename To>
class LocalizationMapper 
    : public unary_function<void, From>
{
    const std::list<To>& spots;
    const boost::ptr_array< binning::Unscaled, 2 >& binners;
    typedef std::vector<From> Store;
  public:
    typedef boost::ptr_map<const To*, Store > Map;

    LocalizationMapper(const std::list<To>& spots, const boost::ptr_array< binning::Unscaled, 2 >& binners)
        : spots(spots), binners(binners)
    {
    }

    void operator()(const From& loc)
    {
        double minDist = std::numeric_limits<double>::max();
        const To* minCand = NULL;
        float locpos[2];
        for (int i = 0; i < 2; ++i) {
            boost::optional<float> v = binners[i].bin_point(loc);
            if ( ! v ) return;
            locpos[i] = *v;
        }
        BOOST_FOREACH( const Spot& spot, spots )
        {
            double dist = sq(spot.position().x().value() - locpos[0]) +
                            sq(spot.position().y().value() - locpos[1]);
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
    const dStorm::Image<float,2>& image = bins();
    candidates.fillMax(image);
    std::list<Spot> foundSpots;
    for (dStorm::engine::CandidateTree<float>::const_iterator
            it = candidates.begin(); it != candidates.end(); ++it)
    {
        foundSpots.push_back( Spot( it->spot() ) );
    }

    Mapper mapper(foundSpots, binners);
    for (const Localization& localization : points) {
        mapper(localization);
    }

    Filter::announce_run(RunAnnouncement());

    int group = 0;
    for (const auto& entry : mapper.getMapping()) {
        EngineResult engineResult;
        engineResult.group = group++;
        std::copy(entry.second->begin(), entry.second->end(),
                  std::back_inserter(engineResult));
        Filter::receiveLocalizations(engineResult);
    }

    Filter::store_children_results( true );
}

void Segmenter::display_image( const ColorImage& img ) {
    if ( announcement.get() == NULL ) return;
    dStorm::display::ResizeChange new_size;
    new_size.set_size( img.sizes() );
    new_size.keys.push_back( dStorm::display::KeyDeclaration( "Segment", "Index of associated region", 1+(sizeof(colors)/sizeof(colors[0])) ));
    for (int i = 0; i < 2; ++i)
        new_size.pixel_sizes[i] = binners[i].resolution();

    if ( display.get() == NULL ) {
        dStorm::display::WindowProperties props;
        props.initial_size = new_size;

        display = user_interface->get_image_window( props, *this );
        next_change.reset( new dStorm::display::Change(1) );
        next_change->changed_keys.front().reserve( new_size.keys.front().size );
        for (int i = 0; i < new_size.keys.front().size; i++)
            next_change->changed_keys.front().push_back( dStorm::display::KeyChange(
                i, (i == 0) ? dStorm::Pixel::Black() : colors[i-1], i ) );
    } else {
        next_change->do_resize = true;
        next_change->resize_image = new_size;
    }

    next_change->do_change_image = true;
    next_change->image_change.new_image = extend( img, dStorm::display::Image() );
}

std::auto_ptr<dStorm::display::Change> Segmenter::get_changes() {
    std::auto_ptr<dStorm::display::Change> fresh( new dStorm::display::Change(1) );
    boost::lock_guard<boost::mutex> lock(mutex);
    std::swap( fresh, next_change );
    return fresh;
}

void Segmenter::attach_ui_( simparm::NodeHandle at ) {
    listening[0].reset();
    listening[1].reset();
    if ( howToSegment == Region ) {
        listening[0] = threshold.value.notify_on_value_change( boost::bind( &Segmenter::segment_locked, this ) );
        listening[1] = dilation.value.notify_on_value_change( boost::bind( &Segmenter::segment_locked, this ) );
        threshold.attach_ui(at);
        dilation.attach_ui( at );
    }

    user_interface = at;
    Filter::attach_children_ui( at );
}

std::auto_ptr< dStorm::output::OutputSource > make_segmenter_source() {
    return std::auto_ptr< dStorm::output::OutputSource >(
        new dStorm::output::FilterBuilder<Segmenter::Config,Segmenter>()
    );
}

}
}

namespace dStorm {
template void rectangular_erosion<unsigned int, unsigned int>(dStorm::Image<unsigned int, 2> const&, dStorm::Image<unsigned int, 2>&, int, int, int, int);
template void rectangular_dilation<unsigned int, unsigned int>(dStorm::Image<unsigned int, 2> const&, dStorm::Image<unsigned int, 2>&, int, int, int, int);
}
