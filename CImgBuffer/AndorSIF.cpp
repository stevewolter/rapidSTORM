#ifdef HAVE_LIBREADSIF

#define CImgBuffer_SIFLOADER_CPP

#include <read_sif.h>
#include <stdexcept>
#include <cassert>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>

#include <simparm/ChoiceEntry_Impl.hh>

#include <CImg.h>

#include "AndorSIF.h"
#include <CImgBuffer/Source.h>
#include <CImgBuffer/Source_impl.h>
#include <CImgBuffer/ImageTraits.h>
#include "foreach.h"
#include <AndorCamera/Config.h>
#include "BasenameWatcher.h"

using namespace std;
using namespace cimg_library;

namespace CImgBuffer {
namespace AndorSIF {

template<typename Pixel>
CImg<Pixel>*
Source<Pixel>::load()
 
{
   const int sz = _height * _width;
   float buffer[sz];
   for (int i = 0; i < sz; i++) buffer[i] = 5;
   CImg<Pixel> *result = NULL;

#ifndef NDEBUG
   int rv_of_readsif_getImage = 
#endif
        readsif_getNextImage( dataSet, buffer );
   assert( rv_of_readsif_getImage == 0 );

   result = new CImg<Pixel>(_width, _height);
   /* The pixel might need casting. This is done here. */
   for (int p = 0; p < sz; p++) {
      result->data[p] = (Pixel)buffer[p];
   }
   return result;
}

template<typename Pixel>
void Source<Pixel>::init(FILE *src)
{
    /* Open the SIF file and find a readable data set. */
    dataSet = NULL;
    stream = src;
    file = readsif_open_File(src);
    if (file == NULL) 
        throw std::runtime_error(file_ident + "is no valid SIF file");

    dataSet = readsif_read_DataSet(file);
    if (dataSet == NULL)  {
        string sif_error_if_any = (readsif_error[0] == 0) ? "" :
            (" " + string(readsif_error));
        throw std::runtime_error(file_ident + " contained no data or could not be read." + sif_error_if_any);
    }
    else if ( readsif_numberOfSubimages(dataSet) > 1 )
        clog << "Warning: SIF file contains multiple subimages. This "
                "feature is not supported and only the first subimage "
                "will be used." << endl;

   _width = readsif_imageWidth( dataSet, 0 );
   _height = readsif_imageHeight( dataSet, 0 );

    /* Read the additional information file from the SIF file
     * and store it in SIF info structure. */
    AndorCamera::Config *sifInfo = new AndorCamera::Config;
    this->sifInfo.reset(sifInfo);
    sifInfo->targetTemperature = dataSet->instaImage.temperature;
    sifInfo->targetTemperature.editable = false;
    sifInfo->outputAmp = 
        (AndorCamera::OutputAmp)dataSet->instaImage.OutputAmp;
    sifInfo->outputAmp.editable = false;

    {
        stringstream ss;
        ss << dataSet->instaImage.data_v_shift_speed*1E6 << " µs";
        sifInfo->VS_Speed.addChoice(-1, "SIFSpeed", ss.str());
        sifInfo->VS_Speed.editable = false;
        sifInfo->VS_Speed.userLevel = simparm::Entry::Expert;
    }
    {
        stringstream ss;
        ss << 1E-6/dataSet->instaImage.pixel_readout_time << " MHz";
        sifInfo->HS_Speed.addChoice(-1, "SIFSpeed", ss.str());
        sifInfo->HS_Speed.editable = false;
        sifInfo->HS_Speed.userLevel = simparm::Entry::Expert;
    }
    sifInfo->emccdGain = dataSet->instaImage.PreAmpGain;
    sifInfo->emccdGain.editable = false;
    sifInfo->realExposureTime = dataSet->instaImage.exposure_time;
    sifInfo->realExposureTime.editable = false;
    sifInfo->cycleTime = dataSet->instaImage.kinetic_cycle_time;
    sifInfo->cycleTime.editable = false;

    simparm::Entry* whn[] = {
        new simparm::UnsignedLongEntry("ImageWidth", "Image width", _width),
        new simparm::UnsignedLongEntry("ImageHeight", "Image height", _height),
        new simparm::UnsignedLongEntry("ImageNumber", "Number of images",
            readsif_numberOfImages(dataSet) ) 
    };

    int n = sizeof(whn) / sizeof(simparm::StringEntry*);
    for (int i = 0; i < n; i++) {
        whn[i]->editable = false;
        sifInfo->manage( whn[i] );
        sifInfo->push_back( *whn[i] );
    }

    receive_changes_from( showDetails );
    receive_changes_from( hideDetails );

    hideDetails.trigger();
}

template<typename Pixel>
Source<Pixel>::Source(FILE *src, const string &i)
: Object("AndorSIF", "SIF file"),
  BaseSource(BaseSource::Pullable),
  Set("AndorSIF", "SIF file"),
  stream(NULL), file(NULL), dataSet(NULL), file_ident(i),
  showDetails("ShowDetails", "Show SIF file information"),
  hideDetails("HideDetails", "Hide SIF file information")
{
   init(src);
}

template<typename Pixel>
Source<Pixel>::Source(const char *filename) 
: SerialImageSource< Pixel >
    (BaseSource::Pushing | BaseSource::Pullable),
  Set("AndorSIF", "SIF file"),
  stream(NULL), file(NULL), dataSet(NULL), file_ident(filename),
  showDetails("ShowDetails", "Show SIF file information"),
  hideDetails("HideDetails", "Hide SIF file information")
{
   FILE *result = fopen(filename, "rb");
   if (result != NULL)
      init(result);
   else {
      throw std::runtime_error((("Could not open " + file_ident) + ": ") +
                         strerror(errno));
    }
}

template<typename Pixel>
Source<Pixel>::~Source() {
    if ( dataSet != NULL )
        readsif_destroy_DataSet( dataSet );
    if ( file != NULL )
        readsif_destroy_File( file );
    if ( stream != NULL ) {
        fclose(stream);
    }
}

template<typename Pixel>
void Source<Pixel>::operator()(Node& source, Cause, Node*) {
    if ( &source == &showDetails && showDetails.triggered() ) {
        showDetails.untrigger();
        if ( sifInfo.get() ) sifInfo->viewable = true;
        showDetails.viewable = false;
        hideDetails.viewable = true;
    } else if ( &source == &hideDetails && hideDetails.triggered() ) {
        hideDetails.untrigger();
        if ( sifInfo.get() ) sifInfo->viewable = false;
        showDetails.viewable = true;
        hideDetails.viewable = false;
    }
    
}

template<typename Pixel>
int Source<Pixel>::quantity() const {
   return readsif_numberOfImages(dataSet);
}

template<typename Pixel>
Source< Pixel >*
Config<Pixel>::impl_makeSource()
{
    Source<Pixel>* ptr =
        new Source<Pixel>(inputFile().c_str());
    ptr->push_back( inputFile );
    ptr->push_back_SIF_info();
    inputFile.editable = false;
    return ptr;
}

template<typename Pixel>
Config<Pixel>::Config( CImgBuffer::Config& src) 
: InputConfig< CImg<Pixel> >("AndorSIF", "Andor SIF file"),
  master(src),
  inputFile(src.inputFile),
  sif_extension("extension_sif", ".sif")
{
    this->register_entry(&inputFile);
    this->register_entry(&master.firstImage);
    this->register_entry(&master.lastImage);
    this->register_entry(&master.pixel_size_in_nm);
    inputFile.push_back(sif_extension);
}

template<typename Pixel>
Config<Pixel>::Config(
    const Config<Pixel>::Config &c,
    CImgBuffer::Config& src
) 
: simparm::Node(c),
  InputConfig< CImg<Pixel> >(c),
  master(src),
  inputFile(src.inputFile),
  sif_extension(c.sif_extension)
{
    this->register_entry(&inputFile);
    this->register_entry(&master.firstImage);
    this->register_entry(&master.lastImage);
    inputFile.push_back(sif_extension);
}

template class Config<unsigned short>;
template class Config<unsigned int>;
template class Config<float>;

}
}

#endif
