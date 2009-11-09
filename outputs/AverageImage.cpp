#define DSTORM_AVERAGEIMAGE_CPP
#include "AverageImage.h"
#include <iostream>
#include "doc/help/context.h"

using namespace std;
namespace dStorm {
namespace output {

AverageImage::_Config::_Config()
: Object("AverageImage", "Average images"),
    outputFile("ToFile", "Write averaged image to")
{ 
    userLevel = Intermediate;
    outputFile.default_extension = ".jpg"; 
    outputFile.helpID = HELP_AverageImage_ToFile;
}

AverageImage::AverageImage( const Config &config )
: simparm::Object("AverageImage", "Image averaging status"),
  filename(config.outputFile()) {}

AverageImage* AverageImage::clone() const 
    { return new AverageImage(*this); }

Output::Result
AverageImage::receiveLocalizations(const EngineResult& er) {
    ost::MutexLock lock(mutex);
    for (unsigned int j = 0; j < image.size(); j++)
        image.data[j] += er.source->data[j];

    return KeepRunning;
}

void AverageImage::propagate_signal(Output::ProgressSignal s)
 
{
    if (s == Job_finished_successfully) {
        ost::MutexLock lock(mutex);
        try {
            if (filename != "")
                image.get_normalize(0,255).save(filename.c_str());
        } catch (const cimg_library::CImgException& e) {
            cerr << "CImg: " << e.message << endl;
        }
    } else if (s == Engine_run_failed) {
        ost::MutexLock lock(mutex);
        image.fill(0);
    }
}


}
}
