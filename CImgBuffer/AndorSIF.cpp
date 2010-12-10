#define CImgBuffer_SIFLOADER_CPP

#include <read_sif.h>
#include <stdexcept>
#include <cassert>
#include <errno.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <functional>

#include "AndorSIF.h"
#include "Pixel.h"
#include "foreach.h"
#include <AndorCamera/Config.h>

using namespace std;

namespace CImgBuffer {

template<>
auto_ptr< Image<Pixel> >
AndorSIFSource<Pixel>::get_image(int image_index)
   throw(std::exception) 
{
   int sz = _height * _width;
   float buffer[sz];
   for (int i = 0; i < sz; i++) buffer[i] = 5;
   Image<Pixel> *result = NULL;

#ifndef NDEBUG
   int rv_of_readsif_getImage = 
#endif
        readsif_getImage( dataSet, image_index, buffer );
   assert( rv_of_readsif_getImage == 0 );
   result = new Image<Pixel>(_width, _height, image_index);
   /* The pixel might need casting. This is done here. */
   for (int p = 0; p < sz; p++) {
      result->data[p] = (Pixel)buffer[p];
   }
   return auto_ptr< Image<Pixel> >(result);
}

template<>
void AndorSIFSource<Pixel>::init(FILE *src) throw(std::exception)
{
    dataSet = NULL;
    stream = src;
    file = readsif_open_File(src);
    if (file == NULL) 
        throw std::runtime_error(file_ident + "is no valid SIF file");

    dataSet = readsif_read_DataSet(file);
    while (dataSet == NULL && readsif_mightContainMoreDataSets(file))
        dataSet = readsif_read_DataSet(file);
    if (dataSet == NULL) 
        throw std::runtime_error(file_ident + " contained no data");
    else if ( readsif_numberOfSubimages(dataSet) > 1 )
        clog << "Warning: SIF file contains multiple subimages. This "
                "feature is not supported and only the first subimage "
                "will be used." << endl;

   _width = readsif_imageWidth( dataSet, 0 );
   _height = readsif_imageHeight( dataSet, 0 );

    AndorCamera::Config *sifInfo = new AndorCamera::Config;
    this->sifInfo.reset(sifInfo);
    sifInfo->targetTemperature = dataSet->instaImage.temperature;
    sifInfo->outputAmp = 
        (AndorCamera::OutputAmp)dataSet->instaImage.OutputAmp;

    {
        stringstream ss;
        ss << dataSet->instaImage.data_v_shift_speed*1E6 << " µs";
        sifInfo->VS_Speed.addChoice(-1, "SIFSpeed", ss.str());
    }
    {
        stringstream ss;
        ss << 1E-6/dataSet->instaImage.pixel_readout_time << " MHz";
        sifInfo->HS_Speed.addChoice(-1, "SIFSpeed", ss.str());
    }
    sifInfo->emccdGain = dataSet->instaImage.PreAmpGain;
    sifInfo->realExposureTime = dataSet->instaImage.exposure_time;
    sifInfo->cycleTime = dataSet->instaImage.kinetic_cycle_time;

    std::for_each( sifInfo->begin(), sifInfo->end(),
        bind1st( mem_fun( &ConfigNode::register_entry ), this ) );
}

template<>
AndorSIFSource<Pixel>::AndorSIFSource(FILE *src, const string &i)
   throw(std::exception) 
: ConfigObject("AndorSIF", "SIF file"),
  ConfigSet("AndorSIF", "SIF file"),
  stream(NULL), file(NULL), dataSet(NULL), file_ident(i)
{
   init(src);
}

template<>
AndorSIFSource<Pixel>::AndorSIFSource(const char *filename) 
    throw(std::exception)
: ConfigObject("AndorSIF", "SIF file"), ConfigSet("AndorSIF", "SIF file"),
  stream(NULL), file(NULL), dataSet(NULL), file_ident(filename)
{
   FILE *result = fopen(filename, "rb");
   if (result != NULL)
      init(result);
   else {
      throw std::runtime_error((("Could not open " + file_ident) + ": ") +
                         strerror(errno));
    }
}

template<>
AndorSIFSource<Pixel>::~AndorSIFSource() throw() {
    if ( dataSet != NULL )
        readsif_destroy_DataSet( dataSet );
    if ( file != NULL )
        readsif_destroy_File( file );
    if ( stream != NULL ) {
        fclose(stream);
    }
}

template<>
int AndorSIFSource<Pixel>::image_quantity() throw() {
   return readsif_numberOfImages(dataSet);
}

template<>
auto_ptr< Source< Image<Pixel> > > 
AndorSIFConfig<Pixel>::makeSource() throw(std::exception)
{
    AndorSIFSource<Pixel>* ptr =
        new AndorSIFSource<Pixel>(inputFile().c_str());
    ptr->push_back( inputFile );
    return auto_ptr< Source< Image<Pixel> > >(ptr);
}

#define NAME "AndorSIF"
#define DESC "Andor SIF file"

template<>
AndorSIFConfig<Pixel>::AndorSIFConfig(
    CImgBuffer::Config< Image<Pixel> >& src) throw() 
: ConfigObject(NAME, DESC),
  master(src),
  inputFile(src.inputFile)
{
    register_entry(&inputFile);
    register_entry(&master.firstImage);
    register_entry(&master.lastImage);
}

template<>
AndorSIFConfig<Pixel>::AndorSIFConfig(
    const AndorSIFConfig<Pixel>::AndorSIFConfig &c,
    CImgBuffer::Config< Image<Pixel> >& src
) throw() 
: ConfigObject(c),
  InputConfig< Image<Pixel> >(c),
  master(src),
  inputFile(src.inputFile)
{
    register_entry(&inputFile);
    register_entry(&master.firstImage);
    register_entry(&master.lastImage);
}

template class AndorSIFSource<Pixel>;
template class AndorSIFConfig<Pixel>;

}
