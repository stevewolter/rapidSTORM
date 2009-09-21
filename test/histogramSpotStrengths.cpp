#include "SpotFitter.h"
#include "locprec/Fluorophore.h"
#include "locprec/NoiseGenerator.h"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_statistics_double.h>
#include "Variance.h"
#include "SigmaGuesser.h"
#include "foreach.h"
#include <dStorm/Image.h>
#include <gsl/gsl_statistics.h>
#include "ColorImage.h"
#include "MaximumList.h"
#include <dStorm/EngineDebug.h>
#include <data-c++/Vector.h>
#include "../src/Garage.h"
#include <dStorm/Car.h>
#include <map>
#include "CandidateScanner.h"

using namespace std;
using namespace dStorm;
using namespace libfitpp;

class SpotHistogram : public CandidateScanner {
  private:
    void engineStarted(int) throw() {}
    void engineRestarted(int) throw() { countMap.clear(); }
    map<int,pair<int,int> > countMap;

    map<int,pair<int,int> >::iterator get(dStorm::SmoothedPixel intensity)
        throw() 
    {
       return countMap.insert( make_pair(intensity/1000, make_pair(0,0)) ).first;
    }
    void good(const dStorm::Maximum& max) throw() {
        get(max.first)->second.first++;
    }
    void bad(const dStorm::Maximum& max) throw() {
        get(max.first)->second.second++;
    }
    void notry(const dStorm::Maximum& max) throw() {
        get(max.first)->second.second++;
    }

  public:
    SpotHistogram(dStorm::CarConfig &config) : CandidateScanner(config) {}
    void print(std::ostream &out) {
        for (map<int,pair<int,int> >::iterator i = countMap.begin(); i!= countMap.end();
            i++) 
        {
            out << i->first << " " << i->second.first << " " << i->second.second << "\n";
        }
    }
};

SpotHistogram* histogram = NULL;

static void carCallback(dStorm::CarConfig &c, dStorm::Carburettor&, 
    dStorm::Crankshaft& cs) throw()
{
    histogram = new SpotHistogram(c);
    cs.addEngineView(*histogram);
}

int main(int argc, char *argv[]) throw() {
    dStorm::CarConfig inconfig;
    dStorm::Car::carStartCallback = carCallback;
    inconfig.registerEntries(inconfig);
    inconfig.readConfig(argc, argv);
    dStorm::Car car(inconfig);
    car.run();
    if (histogram) {
        histogram->print(cout);
        delete histogram;
    }
}
