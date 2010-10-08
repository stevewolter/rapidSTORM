#include "AndorSIF_OpenFile.h"

namespace dStorm {
namespace input {
namespace AndorSIF {

OpenFile::OpenFile(const std::string& filename)
: close_stream_when_finished(true),
  stream(NULL), file(NULL), dataSet(NULL), had_errors(false), 
  file_ident(i)
{
   FILE *result = fopen(filename, "rb");
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
        string sif_error_if_any = (readsif_error[0] == 0) ? "" :
            (" " + string(readsif_error));
        throw std::runtime_error(file_ident + " contained no data or could not be read." + sif_error_if_any);
    }
    else if ( readsif_numberOfSubimages(dataSet) > 1 )
        clog << "Warning: SIF file contains multiple subimages. This "
                "feature is not supported and only the first subimage "
                "will be used." << endl;

    im_count = readsif_numberOfImages(dataSet);

}

struct Reader : public std::stringstream {
    std::string& target;

    Reader(std::string& target) : target(target) {}
    ~Reader() { target = this->str(); }
};

template <typename PixelType>
boost::shared_ptr< Traits<dStorm::Image<PixelType,2> > >
OpenFile::getTraits()
{
    boost::shared_ptr< Traits<dStorm::Image<PixelType,2> > > 
        rv( new Traits<dStorm::Image<PixelType,2> >() );
    /* Read the additional information file from the SIF file
     * and store it in SIF info structure. */
    stringstream ss;

    rv->size.x() = 
            readsif_imageWidth( dataSet, 0 ) * cs_units::camera::pixel;
    rv->size.y() = readsif_imageHeight( dataSet, 0 )
            * cs_units::camera::pixel;
    rv->speed = 1.0 * cs_units::camera::frame / ( dataSet->instaImage.kinetic_cycle_time
            * boost::units::si::second );
    rv->last_frame =
        (readsif_numberOfImages(dataSet) - 1) * cs_units::camera::frame;

    Reader(rv->infos[DataSetTraits::CameraTemperature]) 
        << int(dataSet->instaImage.temperature) * boost::units::celsius::degrees;

    AndorCamera::OutputAmp output_amp = 
    Reader(rv->infos[DataSetTraits::OutputAmplifierType]) 
        << ((AndorCamera::OutputAmp)dataSet->instaImage.OutputAmp);

    Reader(rv->infos[DataSetTraits::VerticalShiftSpeed]) 
        << dataSet->instaImage.data_v_shift_speed*1E6 << " µs";
    Reader(rv->infos[DataSetTraits::HorizontalShiftSpeed]) 
        << 1E-6/dataSet->instaImage.pixel_readout_time << " MHz";
    Reader(rv->infos[DataSetTraits::PreamplifierGain]) 
        << dataSet->instaImage.PreAmpGain;

    return rv;
}

template<typename PixelType>
std::auto_ptr< dStorm::Image<PixelType,2> >
OpenFile::load_image(int count)
{
    typedef dStorm::Image<PixelType,2> Image;
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
            readsif_getImage( dataSet, count, buffer );
    if ( rv_of_readsif_getImage == -1 ) {
        simparm::Message m("Error in SIF file",
            "Error while reading SIF file: " + std::string(readsif_error)
               + ". Will skip remaining images.", 
               simparm::Message::Warning);
        send(m);
        had_errors = true;
        return result;
    } else if ( rv_of_readsif_getImage == -2 ) {
        simparm::Message m("Error in SIF file",
            "Error while reading SIF file: " + std::string(readsif_error)
               + ". Will skip one image.", 
               simparm::Message::Warning);
        send(m);
        return result;
    } else if ( rv_of_readsif_getImage == 1 ) {
        throw std::logic_error("Too many images read from SIF source");
    }

    DEBUG("Creating new image");
    result.reset( new Image(dim) );
    DEBUG("Created new image");
    /* The pixel might need casting. This is done here. */
    for (int p = 0; p < sz; p++) {
        (*result)[p] = (Pixel)buffer[p];
    }
    DEBUG("Loaded next image");
    return result;
}

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
