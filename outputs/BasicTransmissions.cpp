#define CImgBuffer_TIFFLOADER_CPP
#include "BasicTransmissions.h"

#include "LocalizationFile.h"
// #include "Viewer.h"
// #include "ViewerConfig.h"
#include "LocalizationCounter.h"
#include "ProgressMeter.h"
#include "AverageImage.h"
#include "TraceFilter.h"
#include "LocalizationFilter.h"
#include "Slicer.h"
#include "RawImageFile.h"
#include "PrecisionEstimator.h"

#include <CImgBuffer/Config.h>
#include <CImg.h>
#include <dStorm/LocalizationFileReader.h>
#include <CImgBuffer/AndorSIF.h>
#include <CImgBuffer/TIFF.h>
#ifdef HAVE_LIBATMCD32D
#include <CImgBuffer/AndorDirect.h>
#endif

using namespace std;
using namespace locprec;

namespace dStorm {

void basic_inputs( CImgBuffer::Config* inputConfig ) {
    inputConfig->inputMethod.addChoice( 
        new LocalizationFileReader::Config( *inputConfig ) );
#ifdef HAVE_LIBREADSIF
    inputConfig->inputMethod.addChoice( 
        new CImgBuffer::AndorSIF::Config<StormPixel>( *inputConfig ) );
#endif
    inputConfig->inputMethod.addChoice( 
        new CImgBuffer::TIFF::Config<StormPixel>( *inputConfig ) );
#ifdef HAVE_LIBATMCD32D
    inputConfig->inputMethod.addChoice( 
        new CImgBuffer::AndorDirect::Config( *inputConfig ) );
#endif
    
}

void basic_outputs( dStorm::BasicOutputs* o ) {
    o->addChoice( new LocalizationFile::Source() );
    //o->addChoice( new Viewer::Source() );
    o->addChoice( new ProgressMeter::Source() );
    o->addChoice( new LocalizationCounter::Source() );
    o->addChoice( new AverageImage::Source() );
    o->addChoice( new LocalizationFilter::Source() );
    o->addChoice( new TraceCountFilter::Source() );
    o->addChoice( new Slicer::Source() );
    o->addChoice( new RawImageFile::Source() );
    o->addChoice( new SinglePrecisionEstimator::Source() );
    o->addChoice( new MultiPrecisionEstimator::Source() );
}

}
