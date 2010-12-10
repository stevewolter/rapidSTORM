#define DSTORM_AVERAGEIMAGE_CPP
#include "AverageImage.h"
#include <CImgBuffer/Image.h>
#include <iostream>

using namespace std;
namespace dStorm {

Transmission::Result
AverageImage::receiveLocalizations(const EngineResult& er) throw() {
    for (unsigned int j = 0; j < image.size(); j++)
        image.data[j] += er.source->data[j];

    return KeepRunning;
}

void AverageImage::finish() throw() {
    try {
        if (filename != "")
            image.get_normalize(0,255).save(filename.c_str());
    } catch (const cimg_library::CImgException& e) {
        cerr << "CImg: " << e.message << endl;
    }
}


}
