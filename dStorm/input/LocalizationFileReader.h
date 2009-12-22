#ifndef DSTORM_LOCALIZATION_FILE_READER_H
#define DSTORM_LOCALIZATION_FILE_READER_H

#include <fstream>
#include <string>
#include "Method.h"
#include <dStorm/Localization.h>
#include <dStorm/engine/Image.h>
#include <dStorm/output/TraceReducer.h>
#include "LocalizationTraits.h"
#include <dStorm/data-c++/Vector.h>

namespace dStorm {
namespace input {

/** This namespace provides a source that can read STM files.
 *  Standard Source semantics apply. */

namespace LocalizationFileReader {
    struct STM_File {
        std::istream& input;
        Traits<Localization> traits;
        int number_of_fields;

        STM_File(std::istream& input) : input(input) {}
    };

    class Source 
    : public simparm::Object, public input::Source<Localization>
    {
        int level;
        STM_File file;
        data_cpp::Vector< Localization > buffer;
        std::vector< output::Trace > trace_buffer;
        std::auto_ptr<output::TraceReducer> reducer;

        int number_of_newlines();
        void read_localization(Localization& target, int level, 
                               int& use_trace_buffer );
        Localization* fetch(int);

      public:
        Source(const STM_File& file, 
               std::auto_ptr<output::TraceReducer> reducer);

        simparm::Node& getNode() { return *this; }
        const simparm::Node& getNode() const { return *this; }

        virtual int quantity() const 
            { throw std::logic_error("Number of localizations in file "
                    "not known a priori."); }
    };

    class Config 
    : public Method<Localization>
    {
      private:
        input::Config& master;
        output::TraceReducer::Config trace_reducer;
        simparm::Attribute<std::string> stm_extension, txt_extension;
        
      public:
        Config(input::Config& master);
        ~Config();

        static std::auto_ptr<Source> read_file( simparm::FileEntry& name );

      protected:
        Source* impl_makeSource();
      
      private:
        static STM_File read_header
            (simparm::FileEntry& file);

      public:
        Config* clone(input::Config &newMaster) const 
            { return (new Config(newMaster)); }
    };

}
}
}

#endif
