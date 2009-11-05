#include <dStorm/Transmission.h>
#include <dStorm/engine/Image.h>
#include <dStorm/Car.h>
#include <limits>

using namespace std;
using namespace dStorm;
using namespace cimg_library;

const char *fileName;
class ImageOutput : public dStorm::EngineView {
  private:
    int outputSmoothedImage, msx, msy;

  public:
    ImageOutput(int n, int msx, int msy) 
        : outputSmoothedImage(n), msx(msx), msy(msy) {}
    void engineStarted(int) throw() {}
    void engineView(const dStorm::Image &image, 
                    const dStorm::SmoothedImage &smoothed,
                    const dStorm::CandidateTree<dStorm::SmoothedPixel> &,
                    const dStorm::Localization* , int  ) throw()
    {
    if (int(image.getImageNumber()) == outputSmoothedImage) {
        Image copy(image);
        copy.get_normalize(0,255).save("Original.png");
        CImg<SmoothedPixel> smoothed_copy(smoothed);
        SmoothedPixel mini = numeric_limits<SmoothedPixel>::max();
        for (unsigned int x = msx; x < smoothed_copy.width-msx; x++)
          for (unsigned int y = msy; y < smoothed_copy.width-msy; y++)
            mini = min<SmoothedPixel>(mini, smoothed_copy(x,y));
        for (int x = 0; x < msx; x++)
          cimg_forY(smoothed_copy,y) {
            smoothed_copy(x,y) = mini;
            smoothed_copy(smoothed_copy.width-1-x,y) = mini;
        }
        for (int y = 0; y < msy; y++)
          cimg_forX(smoothed_copy,x) {
            smoothed_copy(x,y) = mini;
            smoothed_copy(x,smoothed_copy.height-1-y) = mini;
          }
        smoothed_copy.get_normalize(0,255).save(fileName);
        cerr << "Smoothed image written to " << fileName << endl;
    }
    }
};

static int number;

static void carCallback(dStorm::CarConfig &c, dStorm::Carburettor&, 
    dStorm::Crankshaft& cs) throw()
{
    cs.addEngineView(*(new ImageOutput(number, c.x_maskSize(), c.y_maskSize())));
}

int main(int argc, char *argv[]) throw() {
    EntryUnsignedLong imageNumber("ImageNumber", "");
    EntryString filename("FileName", "", "Smoothed.png");
    dStorm::CarConfig inconfig;
    inconfig.register_entry(&imageNumber);
    inconfig.register_entry(&filename);
    dStorm::Car::carStartCallback = carCallback;
    inconfig.registerEntries(inconfig);
    inconfig.readConfig(argc, argv);
    number = imageNumber();
    fileName = filename().c_str();
    dStorm::Car car(inconfig);
    car.run();
}
