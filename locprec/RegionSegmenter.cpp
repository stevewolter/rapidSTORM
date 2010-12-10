#include "RegionSegmenter.h"
#include "foreach.h"


using namespace std;
using namespace cimg_library;
using namespace dStorm;

namespace locprec {

Segmenter::Segmenter(
    SegmentationType howToSegment,
    double bin_size,
    std::auto_ptr<dStorm::Transmission> output,
    std::auto_ptr<dStorm::TraceReducer> reducer) throw()
: ConfigObject("Segmenter", "Region segmenter"),
  howToSegment( howToSegment ),
  res_enh( 1 / bin_size ),
  points(), filler( &points ), bins( res_enh ),
  output(output),
  reducer(reducer)
{
    add( filler );
    add( bins );
}

void Segmenter::segment() throw()
{
    const CImg<float>& src = bins();

    CImg< int > labImage(src.width, src.height, src.dimz(), 
                                src.dimv());
    int curLab = -1;
    std::vector<int> labArray;

    for (int y = 0; y < int(src.height); y++)
      for (int x = 0; x < int(src.width); x++) {
        labImage(x,y) = -1;
        for (int cy = 0; cy >= -1 && y+cy > 0; cy--) {
          int dy = y+cy;
          for (int dx = max<int>(0, x+cy); dx <= x-cy && 
                   dx < int(src.width); dx++) 
            if ( (src(x,y) > 0) == (src(dx,dy) > 0) ) {
                if ( labImage(x,y) == -1 ) {
                    labImage(x,y) = labImage(dx,dy);
                } else if ( labImage(x,y) > labImage(dx,dy) ) {
                    labArray[labImage(x,y)] = labImage(dx,dy);
                } else if ( labImage(x,y) < labImage(dx,dy) ) {
                    labArray[labImage(dx,dy)] = labImage(x,y);
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

    std::vector<dStorm::Trace*> srMap(cLAi, (dStorm::Trace*)NULL);
    std::vector<int> pixelCounts(cLAi, 0);
    Localization::Raster r = Localization::getRaster(res_enh);
    std::list<dStorm::Trace> regions;

    foreach_const( fit, Localizations, points ) {
        int region = compressedLabArray
            [labImage(fit->getXCenter(r),fit->getYCenter(r))];
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
    for ( i = regions.begin(); i != regions.end(); ) {
        reducer->reduce_trace_to_localization( *i, results.allocate() );
        results.commit();
    }
    EngineResult engineResult;
    engineResult.forImage = -1;
    engineResult.first = results.ptr();
    engineResult.number = results.size();

    output->receiveLocalizations(engineResult);
    output->finish();
}

}
