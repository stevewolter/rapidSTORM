#define CImgBuffer_SIFLOADER_CPP
#include "debug.h"
#include <simparm/Message.hh>
#include <read_sif.h>
#include "AndorSIF_OpenFile.h"
#include <stdexcept>
#include <sstream>
#include <errno.h>
#include <boost/units/systems/si/time.hpp>
#include <boost/units/io.hpp>
#include <boost/units/systems/temperature/celsius.hpp>
#include <dStorm/DataSetTraits.h>
#include <dStorm/ImageTraits.h>
#include <boost/smart_ptr/scoped_array.hpp>

namespace dStorm {
namespace input {
namespace AndorSIF {

OpenFile::OpenFile(const std::string& filename)
: close_stream_when_finished(true),
  stream(NULL), file(NULL), dataSet(NULL), had_errors(false), 
  file_ident(filename)
{
   FILE *result = fopen(filename.c_str(), "rb");
   if (result != NULL)
      init(result);
   else {
      throw std::runtime_error((("Could not open " + file_ident) + ": ") +
                         strerror(errno));
   }
}

void OpenFile::init(FILE *src)
{
    /* Open the SIF file and find a readable data set. */
    dataSet = NULL;
    stream = src;
    file = readsif_open_File(src);
    if (file == NULL) 
        throw std::runtime_error(file_ident + "is no valid SIF file");

    dataSet = readsif_read_DataSet(file);
    if (dataSet == NULL)  {
        std::string sif_error_if_any = (readsif_error[0] == 0) ? "" :
            (" " + std::string(readsif_error));
        throw std::runtime_error(file_ident + " contained no data or could not be read." + sif_error_if_any);
    }
    else if ( readsif_numberOfSubimages(dataSet) > 1 )
        std::clog << "Warning: SIF file contains multiple subimages. This "
                "feature is not supported and only the first subimage "
                "will be used." << std::endl;

    im_count = readsif_numberOfImages(dataSet);

}

struct Reader : public std::stringstream {
    simparm::optional<std::string>& target;

    Reader(simparm::optional<std::string>& target) : target(target) {}
    operator std::ostream&() { return *this; }
    std::ostream& stream() { return *this; }
    ~Reader() { target = this->str(); }
};

template <typename PixelType>
std::auto_ptr< Traits<dStorm::Image<PixelType,3> > >
OpenFile::getTraits()
{
    std::auto_ptr< Traits<dStorm::Image<PixelType,3> > > 
        rv( new Traits<dStorm::Image<PixelType,3> >() );
    /* Read the additional information file from the SIF file
     * and store it in SIF info structure. */
    std::stringstream ss;

    rv->size.x() = 
            readsif_imageWidth( dataSet, 0 ) * camera::pixel;
    rv->size.y() = readsif_imageHeight( dataSet, 0 )
            * camera::pixel;
    rv->size.z() = 1 * camera::pixel;
    if ( dataSet->instaImage.kinetic_cycle_time > 1E-8 ) {
        rv->image_number().resolution() = 1.0f * camera::frame / ( dataSet->instaImage.kinetic_cycle_time
                * boost::units::si::second );
    }
    rv->image_number().range().second =
        (readsif_numberOfImages(dataSet) - 1) * camera::frame;

    boost::units::quantity<boost::units::celsius::temperature,int> temp
        = (int(dataSet->instaImage.temperature) * boost::units::celsius::degrees);
    Reader r(rv->infos[DataSetTraits::CameraTemperature]);
    static_cast<std::ostream&>(r) << temp;

    if ( dataSet->instaImage.OutputAmp == 0 )
        rv->infos[DataSetTraits::OutputAmplifierType] = "Electron multiplication";
    else
        rv->infos[DataSetTraits::OutputAmplifierType] = "Conventional amplification";

    Reader(rv->infos[DataSetTraits::VerticalShiftSpeed]) 
        << dataSet->instaImage.data_v_shift_speed*1E6 << " µs";
    Reader(rv->infos[DataSetTraits::HorizontalShiftSpeed]) 
        << 1E-6/dataSet->instaImage.pixel_readout_time << " MHz";
    Reader(rv->infos[DataSetTraits::PreamplifierGain]) 
        << dataSet->instaImage.PreAmpGain;

    return rv;
}

template<typename PixelType>
std::auto_ptr< dStorm::Image<PixelType,3> >
OpenFile::load_image(int count, simparm::Node& node)
{
    typedef dStorm::Image<PixelType,3> Image;
    DEBUG("Loading next image");
    std::auto_ptr< Image > result;
    if ( had_errors )
        return result;
    const int sz = readsif_imageSize(dataSet);
    boost::scoped_array<float> buffer( new float[sz] );
    for (int i = 0; i < sz; i++) buffer[i] = 5;
    typename Image::Size dim;
    dim.fill(1 * camera::pixel);
    dim.x() = readsif_imageWidth(dataSet, 0) 
        * camera::pixel;
    dim.y() = readsif_imageHeight(dataSet, 0)
        * camera::pixel;

    DEBUG("Calling GetNextImage");
    int rv_of_readsif_getImage = 
            readsif_getImage( dataSet, count, buffer.get() );
    if ( rv_of_readsif_getImage == -1 ) {
        simparm::Message m("Error in SIF file",
            "Error while reading SIF file: " + std::string(readsif_error)
               + ". Will skip remaining images.", 
               simparm::Message::Warning);
        node.send(m);
        had_errors = true;
        return result;
    } else if ( rv_of_readsif_getImage == -2 ) {
        simparm::Message m("Error in SIF file",
            "Error while reading SIF file: " + std::string(readsif_error)
               + ". Will skip one image.", 
               simparm::Message::Warning);
        node.send(m);
        return result;
    } else if ( rv_of_readsif_getImage == 1 ) {
        throw std::logic_error("Too many images read from SIF source");
    }

    DEBUG("Creating new image");
    result.reset( new Image(dim) );
    DEBUG("Created new image");
    /* The pixel might need casting. This is done here. */
    for (int p = 0; p < sz; p++) {
        (*result)[p] = (PixelType)buffer[p];
    }
    DEBUG("Loaded next image");
    return result;
}

template 
std::auto_ptr< Traits<dStorm::Image<unsigned short,3> > >
OpenFile::getTraits();
template 
std::auto_ptr< dStorm::Image<unsigned short,3> >
OpenFile::load_image(int, simparm::Node&);

OpenFile::~OpenFile() {
    if ( dataSet != NULL )
        readsif_destroy_DataSet( dataSet );
    if ( file != NULL )
        readsif_destroy_File( file );
    if ( stream != NULL && close_stream_when_finished ) {
        fclose(stream);
    }
}

}
}
}
