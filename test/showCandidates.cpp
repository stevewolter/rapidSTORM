#include "ColorImage.h"
#include <data-c++/Vector.h>
#include "../src/Garage.h"
#include <dStorm/Car.h>
#include "CandidateScanner.h"

using namespace dStorm;

const dStorm::CarConfig *config;

static EntryUnsignedLong imageNumber("OutputImageNumber", "");

class FitMarker : public CandidateScanner<dStorm::SmoothedPixel> {
  protected:
    auto_ptr<ColorImage> ci;

    virtual void good(const Candidate& max) throw() {
        ci->mark(max.second, ColorImage::Green);
    }
    virtual void bad(const Candidate& max) throw() {
        ci->mark(max.second, ColorImage::Yellow);
    }
    virtual void notry(const Candidate& max) throw() {
        ci->mark(max.second, ColorImage::Cyan);
    }
    bool prepareImage(const dStorm::Image &im) throw() { 
        if (im.getImageNumber() == imageNumber()) {
            ci.reset(new ColorImage(im));
            return true;
        } 
        return false;
    }
    void finishImage() throw() {
        ci->get_normalize(0,255).save("Candidates.png");
        cerr << "Candidate image saved as Candidates.png" << endl;
        ci.reset(NULL);
    }
  public:
    FitMarker(dStorm::CarConfig &config) 
        : CandidateScanner<dStorm::SmoothedPixel>(config) {}
    ~FitMarker() throw() {}
};

static void carCallback(dStorm::CarConfig &c, dStorm::Carburettor&, 
    dStorm::Crankshaft& cs) throw()
{
    cs.addEngineView(*(new FitMarker(c)));
}


int main(int argc, char *argv[]) throw() {
    dStorm::CarConfig inconfig;
    inconfig.register_entry(&imageNumber);
    dStorm::Car::carStartCallback = carCallback;
    inconfig.registerEntries(inconfig);
    inconfig.readConfig(argc, argv);
    dStorm::Car car(inconfig);
    config = &car.getConfig();
    car.run();
}
