#ifndef DSTORM_INPUTS_ANDORSIF_OPENFILE_H
#define DSTORM_INPUTS_ANDORSIF_OPENFILE_H

#include <stdio.h> 
#include <string> 
#include <boost/shared_ptr.hpp> 
#include <dStorm/input/Traits.h>

#ifndef CImgBuffer_SIFLOADER_CPP
typedef void readsif_File;
typedef void readsif_DataSet;
#endif

namespace dStorm {
namespace input {
namespace AndorSIF {

class OpenFile : boost::noncopyable {
    const bool close_stream_when_finished;
    FILE *stream;
    readsif_File *file;
    readsif_DataSet *dataSet;

    bool had_errors;
    int im_count;
    std::string file_ident;

    void init(FILE *src);

  public:
    OpenFile(const std::string& filename);
    ~OpenFile();

    const std::string for_file() const { return file_ident; }

    template <typename PixelType> 
        boost::shared_ptr< Traits<dStorm::Image<PixelType,2> > > 
        getTraits();

    template <typename PixelType>
        std::auto_ptr< dStorm::Image<PixelType,2> >
        load_image( int index );
};

}
}
}

#endif
