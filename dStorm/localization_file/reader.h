#ifndef DSTORM_LOCALIZATION_FILE_READER_H
#define DSTORM_LOCALIZATION_FILE_READER_H

#include <iostream>
#include <string>
#include <dStorm/input/FileBasedMethod.h>
#include <dStorm/Localization.h>
#include <dStorm/engine/Image.h>
#include <dStorm/output/TraceReducer.h>
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/data-c++/Vector.h>
#include <boost/ptr_container/ptr_vector.hpp>

#include "fields_decl.h"

namespace dStorm {
namespace LocalizationFile {

/** This namespace provides a source that can read STM files.
 *  Standard Source semantics apply. */

namespace Reader {
    struct File {
        typedef boost::ptr_vector<field::Interface> Fields;
        typedef input::Traits<Localization> Traits;

        std::istream& input;

        File(std::istream& input);
        ~File();
        Traits getTraits() const;
        void read_next(Localization& target);

      private:
        Fields fs;

        void read_classic(const std::string& first_line);
        void read_XML(const std::string& first_line);
    };

    class Source 
    : public simparm::Object, public input::Source<Localization>
    {
      public:
        struct EmptyImageCallback {
            struct EmptyImageInfo { frame_index number; };
            virtual void notice_empty_image( const EmptyImageInfo& ) = 0;
            virtual ~EmptyImageCallback() {}
        };

      private:
        int level;
        File file;
        data_cpp::Vector< Localization > buffer;
        std::vector< output::Trace > trace_buffer;
        std::auto_ptr<output::TraceReducer> reducer;
        typedef File::Traits::Resolution Resolution;
        Resolution user_resolution;
        EmptyImageCallback* empty_image;

        int number_of_newlines();
        void read_localization(Localization& target, int level, 
                               int& use_trace_buffer );
        Localization* fetch(int);

      public:
        Source(const File& file, 
               std::auto_ptr<output::TraceReducer> reducer);

        void set_default_pixel_size(const File::Traits::Resolution::value_type& resolution);
        simparm::Node& getNode() { return *this; }
        const simparm::Node& getNode() const { return *this; }

        virtual int quantity() const 
            { throw std::logic_error("Number of localizations in file "
                    "not known a priori."); }
        
        EmptyImageCallback* setEmptyImageCallback( EmptyImageCallback* );
    };

    class Config 
    : public input::FileBasedMethod<Localization>
    {
      private:
        output::TraceReducer::Config trace_reducer;
        simparm::Attribute<std::string> txt_extension;
        
      public:
        Config(input::Config& master);
        ~Config();

        static std::auto_ptr<Source> read_file( simparm::FileEntry& name );

      protected:
        Source* impl_makeSource();
      
      private:
        static File read_header
            (simparm::FileEntry& file);

      public:
        Config* clone(input::Config &newMaster) const 
            { return (new Config(newMaster)); }
    };

}
}
}

#endif
