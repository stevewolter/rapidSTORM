#include "RegionSegmenter.h"
#include "foreach.h"
#include <limits>
#include <dStorm/engine/Spot.h>
#include <dStorm/engine/dilation.h>
#include <dStorm/engine/CandidateTree.h>

using namespace std;
using namespace cimg_library;
using namespace dStorm;

namespace locprec {

Segmenter::_Config::_Config()
:   simparm::Object("Segmenter", "Segment target image"),
    method("SegmentationMethod", "Method for segmentation"),
    bin_size("SegmentationBinSize", 
             "Bin size for segmentable image", 0.1),
    threshold("SegmentationThreshold","Threshold for regionness", 0),
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
    method["Regions"].push_back( threshold );
    method["Regions"].push_back( dilation );
    method["Regions"].push_back( bin_size );
    method["Regions"].push_back( load_segmentation );
    method["Regions"].push_back( save_segmentation );

    push_back( method );
    push_back( reducer );
    push_back( trace_filter );
}


Segmenter::Segmenter(
    const Config& config,
    std::auto_ptr<dStorm::Output> output
)
: howToSegment( config.method() ),
  res_enh( 1 / config.bin_size() ),
  threshold( config.threshold ),
  dilation( config.dilation ),
  points(), filler( &points ), bins( res_enh ),
  output(output),
  reducer( config.reducer.make_trace_reducer() ),
  load_segmentation( config.load_segmentation() ),
  save_segmentation( config.save_segmentation() ) 
{
    name = "Segmenter";
    desc = "Region segmenter";

    add( filler );
    add( bins );

    if ( howToSegment == Region ) {
        receive_changes_from( this->threshold );
        push_back( this->threshold );
        receive_changes_from( this->dilation );
        push_back( this->dilation );
    }

    push_back( *this->output );
}

Output::AdditionalData Segmenter::announceStormSize
            (const Announcement &a) 
{
    ost::MutexLock lock( mutex );
    /* TODO: Evaluate return */
    output->announceStormSize(a);
    return dStorm::Crankshaft::announceStormSize(a);
}

void Segmenter::operator()(
    simparm::Node& src, simparm::Node::Callback::Cause c, simparm::Node*)

{
    if ( (&src == &threshold || &src == &dilation) && c == ValueChanged )
    {
        ost::MutexLock lock(mutex);
        segment();
    }
}

static uint8_t colors[][3] = {
    { 255, 0, 0 },
    { 0, 255, 0 },
    { 0, 0, 255 },
    { 0, 255, 255 },
    { 255, 0, 255 },
    { 255, 255, 0 },
};

static std::auto_ptr<CImgDisplay> colorful_regions(const CImg<int>& regImage,
    std::auto_ptr<CImgDisplay>& old) 
{
    CImg< uint8_t > coloredImage( regImage.width, regImage.height,1,3);
    cimg_forXY( regImage, x, y ) {
        if ( regImage(x,y) == 0 )
            coloredImage(x,y,0,0) = coloredImage(x,y,0,1)
                = coloredImage(x,y,0,2) = 0;
        else {
            int c = regImage(x,y) % (sizeof(colors)/sizeof(colors[0]));
            for (int d = 0; d < 3; d++)
                coloredImage(x,y,0,d) = colors[c][d];
        }
    }
    if ( old.get() != NULL ) {
        coloredImage.resize( *old );
        old->display( coloredImage );
        return old;
    } else
        return std::auto_ptr<CImgDisplay>
            (new CImgDisplay(coloredImage, "Regions in image"));
}

static void merge(std::vector<int>& labArray, int a, int b) {
    int region = std::min(labArray[a], labArray[b]);
    int merged = std::max(labArray[a], labArray[b]);

    std::replace( labArray.begin(), labArray.end(), merged, region );
}

CImg<int> Segmenter::segment_image()
{
    const CImg<float>& src = bins();
    CImg<bool> thres = src.get_threshold( threshold() ),
               dilated( src.width, src.height, 1, 1 );
    if ( dilation() > 0 )
        dStorm::rectangular_dilation( thres, dilated, 
                                      dilation(), dilation(), 0, 0 );
    else
        dilated = thres;

    CImg< int > labImage(src.width, src.height, src.dimz(), 
                                src.dimv());
    int curLab = 0;
    const int Background = 0;
    std::vector<int> labArray;
    labArray.push_back( Background );

    /* X/Y offset pairs for a forward scan. */
    const int forward_scan[][2] = { {-1,-1}, {0,-1}, {1,-1}, {-1,0} };
    const int forward_scan_offsets = 4;

    for (int y = 0; y < int(src.height); y++)
      for (int x = 0; x < int(src.width); x++) {
        if ( dilated(x,y) == false ) {
            labImage(x,y) = Background;
            continue;
        }

        labImage(x,y) = -1;
        for (int n = 0; n < forward_scan_offsets; n++) {
            int dx = forward_scan[n][0]+x, dy = forward_scan[n][1]+y;
            if ( !dilated.contains(dx,dy) )
                continue;

            if ( dilated(x,y) == dilated(dx,dy) ) {
                if ( labImage(x,y) == -1 ) {
                    labImage(x,y) = labImage(dx,dy);
                } else if ( labImage(x,y) != labImage(dx,dy) ) {
                    merge( labArray, labImage(x,y), labImage(dx,dy) );
                }
            }
        }

        if (labImage(x,y) == -1) {
            curLab++;
            labArray.push_back(curLab);
            labImage(x,y) = curLab;
        }
    }

    int cLAi = 0;
    std::vector<int> compressedLabArray(labArray.size(), -1);
    for (int i = 0; i < int(labArray.size()); i++)
        if ( i == labArray[i] )
            compressedLabArray[i] = cLAi++;
        else
            compressedLabArray[i] = compressedLabArray[labArray[i]];

    cimg_for( labImage, i, int )
        *i = compressedLabArray[*i];

    display = colorful_regions(labImage, display);

    return labImage;
}

void Segmenter::segment()
{
    CImg<int> segmentation;
    if ( load_segmentation != "" )
        segmentation.load( load_segmentation.c_str() );
    else 
        segmentation = segment_image();

    if ( save_segmentation != "" )
        segmentation.save( save_segmentation.c_str() );

    std::vector<dStorm::Trace*> srMap(segmentation.max()+1,
                                      (dStorm::Trace*)NULL);
    Localization::Raster r = Localization::getRaster(res_enh);
    std::list<dStorm::Trace> regions;

    foreach_const( fit, Localizations, points ) {
        int region = segmentation(fit->getXCenter(r),fit->getYCenter(r));
        if (srMap[region] == NULL) {
            regions.push_back( dStorm::Trace() );
            regions.back().push_back( *fit );
            srMap[region] = &regions.back();
        } else {
            dStorm::Trace &sr = *srMap[region];
            sr.push_back(*fit);
        }
    }

    list<dStorm::Trace>::iterator i;
    data_cpp::Vector<dStorm::Localization> results;
    for ( i = regions.begin(); i != regions.end(); i++) {
        reducer->reduce_trace_to_localization( *i, results.allocate(),
            Eigen::Vector2d::Zero() );
        results.commit();
    }
    EngineResult engineResult;
    engineResult.forImage = -1;
    engineResult.first = results.ptr();
    engineResult.number = results.size();

    output->propagate_signal(Engine_is_restarted);
    output->receiveLocalizations(engineResult);
    output->propagate_signal(Engine_run_succeeded);
}

template <typename To> To sq(To a) { return a*a; }

template <typename From, typename To, typename Store>
class LocalizationMapper 
    : public unary_function<void, From>
{
  public:
    typedef map<const To*, Store* > Map;
    typedef pair<const To*, Store* > Pair;

    LocalizationMapper() {}
    LocalizationMapper(const LocalizationMapper&);
    LocalizationMapper& operator=(const LocalizationMapper&);
    ~LocalizationMapper() {
        for (typename Map::iterator i = mapping.begin(); 
                                    i != mapping.end(); i++)
            if ( i->second != NULL ) {
                delete i->second;
                i->second = NULL;
            }
    }

    LocalizationMapper(const list<To>& spots)
    {
        typename list<To>::const_iterator spot;
        for (spot = spots.begin();
                spot != spots.end(); spot++)
        {
            int cx = spot->x()/10, cy = spot->y()/10;
            for (int dx = -1; dx <= 1; dx++)
                for (int dy = -1; dy <= 1; dy++) {
                if (int(cache.size()) <= cy+dy)
                    cache.resize(cy+dy+20);
                if (cy+dy >= 0 && int(cache[cy+dy].size()) <= cx+dx)
                    cache[cy+dy].resize(cx+dx+20);

                if ( cy+dy >= 0 && cx+dx >= 0 )
                    cache[cy+dy][cx+dx].push_back( &*spot );
            }
        }
    }

    void operator()(const From& loc)
    {
        double minDist = std::numeric_limits<double>::max();
        const To* minCand = NULL;
        int cx = (int)loc.x(), cy = (int)loc.y();
        if (cy < 0 || int(cache.size()) <= cy ||
            cx < 0 || int(cache[cy].size()) <= cx) 
        {
            //cout << "Too small cache for " << cy << " " << cx << endl;
            return;
        }
        typename list<const To*>::const_iterator spot;
        for ( spot = cache[cy][cx].begin(); 
                spot != cache[cy][cx].end(); spot++ )
        {
            double dist = sq((*spot)->x()/10.0 - loc.x()) +
                            sq((*spot)->y()/10.0 - loc.y());
            if (dist < minDist) {
                minDist = dist;
                minCand = *spot;
            }
        }
        if (minCand) {
            typename Map::iterator rightStore = mapping.find(minCand);
            if ( rightStore == mapping.end() )
                rightStore = mapping.insert(
                    make_pair(minCand, new Store()) ).first;

            rightStore->second->push_back( loc );
        }
    }

    const Map& getMapping() const { return mapping; }

  private:
    Map mapping;
    vector<vector<list<const To*> > > cache;
};

typedef 
LocalizationMapper<const Localization, dStorm::Spot, dStorm::Trace> Mapper;

void Segmenter::maximums() {
    dStorm::CandidateTree<float> candidates(3,3,0,0);
    candidates.setLimit(100000);
    const cimg_library::CImg<float>& image = bins();
    candidates.fillMax(image);
    list<Spot> foundSpots;
    for (dStorm::CandidateTree<float>::iterator
            it = candidates.begin(); it.hasMore(); it++)
        foundSpots.push_back(it->second);

    dStorm::Localizations& locs = points;
    Mapper mapper(foundSpots), result;
    for (dStorm::Localizations::const_iterator i= locs.begin(); 
                                               i!= locs.end(); i++)
        mapper(*i);

    data_cpp::Vector<Localization> binned_localizations;
    for ( Mapper::Map::const_iterator i = mapper.getMapping().begin();
          i != mapper.getMapping().end(); i++)
    {
        reducer->reduce_trace_to_localization( *i->second,
            binned_localizations.allocate(),
            Eigen::Vector2d::Zero() );
        binned_localizations.commit();
    }

    EngineResult engineResult;
    engineResult.forImage = -1;
    engineResult.first = binned_localizations.ptr();
    engineResult.number = binned_localizations.size();

    output->propagate_signal(Engine_is_restarted);
    output->receiveLocalizations(engineResult);
    output->propagate_signal(Engine_run_succeeded);
}

}
