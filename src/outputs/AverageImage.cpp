#define cimg_use_magick
#define DSTORM_AVERAGEIMAGE_CPP
#include "AverageImage.h"
#include <iostream>
#include "doc/help/context.h"

using namespace std;
namespace dStorm {
namespace output {

AverageImage::_Config::_Config()
: Object("AverageImage", "Average images"),
    outputFile("ToFile", "Write averaged image to", ".jpg")
{ 
    userLevel = Intermediate;
    outputFile.helpID = HELP_AverageImage_ToFile;
}

AverageImage::AverageImage( const Config &config )
: OutputObject("AverageImage", "Image averaging status"),
  filename(config.outputFile()) {}

AverageImage* AverageImage::clone() const 
    { return new AverageImage(*this); }

Output::Result
AverageImage::receiveLocalizations(const EngineResult& er) {
    ost::MutexLock lock(mutex);
    for (unsigned int j = 0; j < image.size(); j++)
#if cimg_version <= 129
        image.data[j] += er.source->data[j];
#else
        image.data()[j] += er.source->data()[j];
#endif

    return KeepRunning;
}

void AverageImage::propagate_signal(Output::ProgressSignal s)
 
{
    if (s == Engine_run_succeeded && filename != "" ) {
        ost::MutexLock lock(mutex);
        try {
            image.get_normalize(0,255).save(filename.c_str());
        } catch (const cimg_library::CImgException& e) {
#if cimg_version <= 129
            cerr << "CImg: " << e.message << endl;
#else
            cerr << "CImg: " << e.what() << endl;
#endif
        }
    } else if (s == Engine_run_failed) {
        ost::MutexLock lock(mutex);
        image.fill(0);
    }
}


}
}
