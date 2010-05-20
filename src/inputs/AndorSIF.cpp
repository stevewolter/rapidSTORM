#include "config.h"
#ifdef HAVE_LIBREADSIF

#include "debug.h"

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

#include "AndorSIF.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/Source_impl.h>
#include <dStorm/ImageTraits.h>
#include <AndorCamera/Config.h>
#include <dStorm/input/BasenameWatcher.h>
#include <dStorm/input/FileBasedMethod_impl.h>

#include <boost/iterator/iterator_facade.hpp>

using namespace std;

namespace dStorm {
namespace input {
namespace AndorSIF {

template<typename Pixel>
std::auto_ptr< typename Source<Pixel>::Image >
Source<Pixel>::load()
{
    DEBUG("Loading next image");
    std::auto_ptr< Image > result;
    if ( had_errors )
        return result;
    const int sz = readsif_imageSize(dataSet);
    float buffer[sz];
    for (int i = 0; i < sz; i++) buffer[i] = 5;
    typename Image::Size dim;
    dim.x() = readsif_imageWidth(dataSet, 0) 
        * cs_units::camera::pixel;
    dim.y() = readsif_imageHeight(dataSet, 0)
        * cs_units::camera::pixel;

    DEBUG("Calling GetNextImage");
    int rv_of_readsif_getImage = 
            readsif_getNextImage( dataSet, buffer );
    if ( rv_of_readsif_getImage == -1 ) {
        std::cerr << "Error while reading SIF file: " + std::string(readsif_error) + ". Will skip remaining images." << std::endl;
        had_errors = true;
        return result;
    } else if ( rv_of_readsif_getImage == 1 ) {
        throw std::logic_error("Too many images read from SIF source");
    }

    DEBUG("Creating new image");
    result.reset( new Image(dim) );
    /* The pixel might need casting. This is done here. */
    for (int p = 0; p < sz; p++) {
        (*result)[p] = (Pixel)buffer[p];
    }
    DEBUG("Loaded next image");
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
        new simparm::UnsignedLongEntry("ImageWidth", "Image width", readsif_imageWidth(dataSet, 0) ),
        new simparm::UnsignedLongEntry("ImageHeight", "Image height", readsif_imageHeight(dataSet, 0) ),
        new simparm::UnsignedLongEntry("ImageNumber", "Number of images",
            readsif_numberOfImages(dataSet) ) 
    };

    int n = sizeof(whn) / sizeof(simparm::Entry*);
    for (int i = 0; i < n; i++) {
        whn[i]->editable = false;
        sifInfo->push_back( std::auto_ptr<simparm::Node>( whn[i] ));
    }

    receive_changes_from( showDetails.value );
    receive_changes_from( hideDetails.value );

    hideDetails.trigger();
}

template<typename Pixel>
Source<Pixel>::Source(FILE *src, const string &i)
: Object("AndorSIF", "SIF file"),
  BaseSource(static_cast<simparm::Node&>(*this), Flags()),
  Set("AndorSIF", "SIF file"),
  has_been_iterated(false),
  stream(NULL), file(NULL), dataSet(NULL), had_errors(false), file_ident(i),
  showDetails("ShowDetails", "Show SIF file information"),
  hideDetails("HideDetails", "Hide SIF file information")
{
   init(src);
}

template<typename Pixel>
Source<Pixel>::Source(const char *filename) 
: Set("AndorSIF", "SIF file"),
  BaseSource
    ( static_cast<simparm::Node&>(*this), Flags() ),
  has_been_iterated(false),
  stream(NULL), file(NULL), dataSet(NULL), had_errors(false), file_ident(filename),
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
void Source<Pixel>::operator()(const simparm::Event& e) {
    if ( &e.source == &showDetails.value && showDetails.triggered() ) {
        showDetails.untrigger();
        if ( sifInfo.get() ) sifInfo->viewable = true;
        showDetails.viewable = false;
        hideDetails.viewable = true;
    } else if ( &e.source == &hideDetails.value && hideDetails.triggered() ) {
        hideDetails.untrigger();
        if ( sifInfo.get() ) sifInfo->viewable = false;
        showDetails.viewable = true;
        hideDetails.viewable = false;
    }
    
}

template<typename Pixel>
typename Source<Pixel>::TraitsPtr 
Source<Pixel>::get_traits()
{
   TraitsPtr rv( new typename TraitsPtr::element_type() );
   rv->size.x() = 
        readsif_imageWidth( dataSet, 0 ) * cs_units::camera::pixel;
   rv->size.y() = readsif_imageHeight( dataSet, 0 )
        * cs_units::camera::pixel;
   rv->size.z() = 1 * cs_units::camera::pixel;

   rv->total_frame_count =
    readsif_numberOfImages(dataSet) * cs_units::camera::frame;
    return rv;
}

template<typename Pixel>
Source< Pixel >*
Config<Pixel>::impl_makeSource()
{
    Source<Pixel>* ptr =
        new Source<Pixel>(this->inputFile().c_str());
    ptr->push_back( this->inputFile );
    ptr->push_back_SIF_info();
    this->inputFile.editable = false;
    return ptr;
}

template<typename Pixel>
Config<Pixel>::Config( input::Config& src) 
: FileBasedMethod< dStorm::Image<Pixel,2> >(src,
    "AndorSIF", "Andor SIF file",
    "extension_sif", ".sif")
{
    this->push_back(src.firstImage);
    this->push_back(src.lastImage);
    this->push_back(src.pixel_size_in_nm);
}

template<typename Pixel>
Config<Pixel>::Config(
    const Config<Pixel>::Config &c,
    input::Config& src
) 
: FileBasedMethod< dStorm::Image<Pixel,2> >(c, src)
{
    this->push_back(src.firstImage);
    this->push_back(src.lastImage);
    this->push_back(src.pixel_size_in_nm);
}

template <typename Pixel>
class Source<Pixel>::iterator
: public boost::iterator_facade<iterator,Image,std::input_iterator_tag>
{
    mutable dStorm::Image<Pixel,2> img;
    Source* src;
    int count;

    friend class boost::iterator_core_access;

    Image& dereference() const { return img; }
    bool equal(const iterator& i) const {
        return (src == i.src) && (src == NULL || count == i.count); 
    }

    void get_next() {
        std::auto_ptr< Image > i = src->load();
        if ( i.get() == NULL ) {
            src = NULL;
            img.invalidate();
        } else {
            img = *i;
            img.frame_number()
                = count++ * cs_units::camera::frame;
        }
    }

    void increment() { get_next(); }
  public:
    iterator() : src(NULL) {}
    iterator(Source& s) : src(&s), count(0)
    {
        if ( s.has_been_iterated )
            throw std::logic_error("SIF source cannot be iterated twice");
        s.has_been_iterated = true;
        
        get_next();
    }
};

template <typename PixelType>
typename Source<PixelType>::base_iterator 
Source<PixelType>::begin() {
    return base_iterator( iterator(*this) );
}
template <typename PixelType>
typename Source<PixelType>::base_iterator 
Source<PixelType>::end() {
    return base_iterator( iterator() );
}

template class Config<unsigned short>;
//template class Config<unsigned int>;
//template class Config<float>;

}
}
}

#endif
