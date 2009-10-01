#ifndef DSTORM_LOCALIZATION_FILE_READER_H
#define DSTORM_LOCALIZATION_FILE_READER_H

#include <fstream>
#include <string>
#include <CImgBuffer/InputMethod.h>
#include <dStorm/engine/Localization.h>

namespace CImgBuffer {
template<>
class Traits<dStorm::Localization> {
  public:
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
        STM_File file;
        Localization buffer;

        Localization* fetch(int);

      public:
        Source(const STM_File& file)
        : CImgBuffer::Source<Localization>
            (BaseSource::Pushing | 
             BaseSource::Pullable | BaseSource::Managing),
          simparm::Object("STM_Show", "Input options"),
          file(file)
        {
            *(CImgBuffer::Traits<Localization>*)this
                = file.traits;
        }
        virtual int quantity() const 
            { throw std::logic_error("Number of localizations in file "
                    "not known a priori."); }
    };

    class Config 
    : public CImgBuffer::InputConfig<Localization>
    {
      private:
        CImgBuffer::Config& master;
        simparm::Attribute<std::string> stm_extension, txt_extension;
        
      public:
        Config(CImgBuffer::Config& master) 
        : CImgBuffer::InputConfig<Localization>
                ("STM", "Localizations file"),
          master(master),
          stm_extension("extension_stm", ".stm"),
          txt_extension("extension_txt", ".txt") 
        {
            this->register_entry(&master.inputFile);
            this->register_entry(&master.firstImage);
            this->register_entry(&master.lastImage);

            master.inputFile.push_back( stm_extension );
            master.inputFile.push_back( txt_extension );
        }

      protected:
        Source* impl_makeSource();
      
      private:
        STM_File read_header
            (simparm::FileEntry& file);

      public:
        Config* clone(CImgBuffer::Config &newMaster) const 
            { return (new Config(newMaster)); }
    };

}
}

#endif
