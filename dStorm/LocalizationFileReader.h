#ifndef DSTORM_LOCALIZATION_FILE_READER_H
#define DSTORM_LOCALIZATION_FILE_READER_H

#include <fstream>
#include <string>
#include <CImgBuffer/InputMethod.h>
#include <dStorm/Localization.h>
#include <dStorm/Image.h>
#include <dStorm/TraceReducer.h>
#include <CImgBuffer/ImageTraits.h>

namespace CImgBuffer {
template<>
class Traits<dStorm::Localization> 
: public CImgBuffer::Traits< dStorm::Image > 
{
  public:
    int dimx() const { return width; }
    int dimy() const { return height; }

    unsigned int width, height, imageNumber;
};
}

namespace dStorm {

/** This namespace provides a source that can read STM files.
 *  Standard CImgBuffer::Source semantics apply. */

namespace LocalizationFileReader {
    struct STM_File {
        std::istream& input;
        CImgBuffer::Traits<Localization> traits;
        int number_of_fields;

        STM_File(std::istream& input) : input(input) {}
    };

    class Source 
    : public CImgBuffer::Source<Localization>, public simparm::Object
    {
        int level;
        STM_File file;
        Localization buffer;
        std::vector< dStorm::Trace > trace_buffer;
        std::auto_ptr<TraceReducer> reducer;

        int number_of_newlines();
        void read_localization(Localization& target, int level, 
                               int& use_trace_buffer );
        Localization* fetch(int);

      public:
        Source(const STM_File& file, 
               std::auto_ptr<TraceReducer> reducer);
        virtual int quantity() const 
            { throw std::logic_error("Number of localizations in file "
                    "not known a priori."); }
    };

    class Config 
    : public CImgBuffer::InputConfig<Localization>
    {
      private:
        CImgBuffer::Config& master;
        TraceReducer::Config trace_reducer;
        simparm::Attribute<std::string> stm_extension, txt_extension;
        
      public:
        Config(CImgBuffer::Config& master);
        ~Config();

        static std::auto_ptr<Source> read_file( simparm::FileEntry& name );

      protected:
        Source* impl_makeSource();
      
      private:
        static STM_File read_header
            (simparm::FileEntry& file);

      public:
        Config* clone(CImgBuffer::Config &newMaster) const 
            { return (new Config(newMaster)); }
    };

}
}

#endif
