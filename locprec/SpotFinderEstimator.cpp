#define LOCPREC_SPOTFINDERESTIMATOR_CPP
#include "SpotFinderEstimator.h"
#include "foreach.h"
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Input.h>
#include <gsl/gsl_statistics.h>
#include <queue>
#include "relate.h"
#include <dStorm/engine/CandidateTree.h>
#include <cassert>

#define SQ(x) ((x)*(x))

using namespace std;
namespace locprec {

SpotFinderEstimator::_Config::_Config()
: simparm::Object("SpotFinderEstimator", 
                  "Estimate spot finder performance"),
  outputFile("ToFile", "Write statistics to") 
{
    userLevel = Expert;
}

SpotFinderEstimator::SpotFinderEstimator(
    Config& config
)
: OutputObject("SpotFinderEstimator", ""),
  fluorophores(NULL), msx(2 * camera::pixel), msy(2 * camera::pixel), numberOfSamples(0),
  noCol(8), out(config.outputFile.get_output_stream())
{
}

SpotFinderEstimator::~SpotFinderEstimator()
{
}

void SpotFinderEstimator::finish()
{
    if (numberOfSamples > 0) {
        /*cerr << "Average spot number / Spot miss rate / True positive fits / "
             << "False positive fits / "
                << "Frustration-corrected spot miss rate "
                << "/ Frustration-corrected false positive average "
                << "/ Strength quotient / Frustration-corrected "
                << "spot hit rate" << endl;*/
        out
            << gsl_stats_mean(&data[0][0], noCol, numberOfSamples)
            << " "
            << gsl_stats_mean(&data[0][1], noCol, numberOfSamples)
            << " "
            << gsl_stats_mean(&data[0][7], noCol, numberOfSamples)
            << " "
            << gsl_stats_mean(&data[0][6], noCol, numberOfSamples)
            << " "
            << gsl_stats_mean(&data[0][2], noCol, numberOfSamples)
            << " "
            << gsl_stats_mean(&data[0][3], noCol, numberOfSamples)
            << " "
            << gsl_stats_wmean(&data[0][1], noCol, &data[0][4], noCol,
                            numberOfSamples)
            << " "
            << gsl_stats_mean(&data[0][5], noCol, numberOfSamples)
            << endl;
    }

}

typedef dStorm::engine::CandidateTree<dStorm::engine::SmoothedPixel> Candidates;

void makeVectorFromCandidates(
    const Candidates &maximums,
    data_cpp::Vector<Optics::ImagePosition>& maximumVector,
    const dStorm::traits::Position&  )
{
    maximumVector.clear();
    for ( Candidates::const_iterator cM = maximums.begin(); cM.hasMore(); ++cM) 
    {
        Optics::ImagePosition p = Optics::ImagePosition::Constant( 0 * camera::pixel ); 
        p.x() = float(cM->x()) * camera::pixel;
        p.y() = float(cM->y()) * camera::pixel;
        maximumVector.push_back(p);
    }
}
   

dStorm::output::Output::Result SpotFinderEstimator::receiveLocalizations
    ( const EngineResult &er )

{
    if ( fluorophores == NULL ) {
        assert( carburettor != NULL );
        const NoiseSource<dStorm::engine::StormPixel>* source = 
            dynamic_cast<const NoiseSource<dStorm::engine::StormPixel>*>(
                                carburettor );
        if (source == NULL) {
            cerr << "Missing source with generated fluorophores for spot finder "
                    "estimation.\n";
            return RemoveThisOutput;
        } else {
            fluorophores = &source->getFluorophores();
        }
    }

    const Candidates &maximums = *er.candidates;
    data_cpp::Vector<Fluorophore::Position> fits;
    for (int i = 0; i < er.number; i++) {
        Fluorophore::Position p;
        for (int j = 0; j < 2; ++j)
            p[j] = Fluorophore::Position::Scalar(er.first[i].position()[j] * *traits.position().resolution()[j]);
        fits.push_back(p);
    }

    int imNo = er.forImage / camera::frame;
    double maximumStrengths[maximums.size()];
    int foundSpots = 0, missedSpots = 0;
    int tpFits = 0;
    Candidates::const_iterator cM = maximums.begin();

    data_cpp::Vector<const Fluorophore*> 
        fluorophoresForFits(er.number, (Fluorophore*)NULL),
        good_maximums(maximums.size(), NULL);
    data_cpp::Vector<Optics::ImagePosition> maximumVector;
    data_cpp::Vector<const Fluorophore*> onFluorophores;

    foreach_const( f, boost::ptr_list<Fluorophore>, *fluorophores )
        if ( f->wasOnInImage(imNo) )
            onFluorophores.push_back(&(*f));

    makeVectorFromCandidates(maximums, maximumVector, traits);
    relate(fits, onFluorophores, fluorophoresForFits, msx, msy);
    relate(maximumVector, onFluorophores, good_maximums, msx, msy);

    for (int i = 0; i < fluorophoresForFits.size(); i++)
        if (fluorophoresForFits[i])
            tpFits++;
    for (int i = 0; i < good_maximums.size(); i++)
        if (good_maximums[i])
            foundSpots++;
    int fluorophoreCount = 0;
    foreach_const( f, boost::ptr_list<Fluorophore>, *fluorophores )
        if ( f->wasOnInImage(imNo) )
            fluorophoreCount++;
    missedSpots = fluorophoreCount - foundSpots;

    {
        int i = 0;
        for ( cM = maximums.begin(); cM.hasMore(); cM++) 
            maximumStrengths[i++] = cM->first;
    }
        
    int best_miss = -1;
    for (int i = 0; i < good_maximums.size(); i++)
        if (good_maximums[i] == NULL && maximumStrengths[i] > 0) {
            best_miss = i;
            break;
        }

    double distances[foundSpots];
    int pos = 0, zeroes = 0;
    for (int curSpot = 0; curSpot < foundSpots; curSpot++) {
        while (good_maximums[pos] == NULL) { pos++; zeroes++; }
        distances[curSpot] = zeroes;
        zeroes = 0;
        pos++;
    }

    double quotients[foundSpots];
    if (best_miss != -1) {
        int curSpot = 0;
        double bestMissStrength = maximumStrengths[best_miss];
        for (int i = 0; i < good_maximums.size(); i++)
            if (good_maximums[i]) {
                quotients[curSpot] = 
                    maximumStrengths[i] / bestMissStrength;
                curSpot++;
                if (curSpot == foundSpots) break;
            }
    }

    int realMissedSpots = foundSpots + missedSpots;
    int realFalseSpots = 0, realTrueSpots = 0;
    int init_frustration = 3, frustration = init_frustration;
    for (int i = 0; i < good_maximums.size(); i++)
        if (good_maximums[i]) {
            realMissedSpots--;
            realTrueSpots++;
            frustration = init_frustration;
        } else {
            realFalseSpots++;
            frustration--;
            if (frustration == 0) break;
        }
#if 0
    for (unsigned int i = 0; i < maximums.size(); i++)
        cerr << good_maximums[i] << " ";
    cerr << endl;
    for (int i = 0; i < foundSpots; i++)
        cerr << distances[i] << " ";
    cerr << endl;
#endif
    ost::MutexLock lock(mutex);

    data[imNo][0] = foundSpots;
    data[imNo][1] = missedSpots;
    data[imNo][2] = realMissedSpots;
    data[imNo][3] = realFalseSpots;
    if (best_miss != -1 && foundSpots > 0) {
        data[imNo][4] = gsl_stats_mean(quotients, 1, foundSpots);
    }
    else
        data[imNo][4] = 1;
    data[imNo][5] = realTrueSpots;
    data[imNo][6] = fits.size() - tpFits;
    data[imNo][7] = tpFits;

    return KeepRunning;
}

}
