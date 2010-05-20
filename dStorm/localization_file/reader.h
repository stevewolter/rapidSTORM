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

        class iterator;
        friend class iterator;

      private:
        typedef std::vector< output::Trace > TraceBuffer;

        int level;
        File file;
        std::auto_ptr<output::TraceReducer> reducer;
        typedef File::Traits::Resolution Resolution;
        Resolution user_resolution;
        EmptyImageCallback* empty_image;

        int number_of_newlines();
        void read_localization(Localization& target, int level, 
                               std::vector<output::Trace>& trace_buffer, 
                               int trace_buffer_index = 0 );

      public:
        Source(const File& file, 
               std::auto_ptr<output::TraceReducer> reducer);

        void set_default_pixel_size(const File::Traits::Resolution::value_type& resolution);
        simparm::Node& getNode() { return *this; }
        const simparm::Node& getNode() const { return *this; }

        EmptyImageCallback* setEmptyImageCallback( EmptyImageCallback* );

        input::Source<Localization>::iterator begin();
        input::Source<Localization>::iterator end();
        TraitsPtr get_traits();
            
    };

    class Config 
    : public input::FileBasedMethod<Localization>
    {
      private:
        output::TraceReducer::Config trace_reducer;
        simparm::Attribute<std::string> txt_extension;

        void registerNamedEntries();
        
      public:
        Config(input::Config& master);
        Config(const Config&, input::Config&);
        ~Config();

        static std::auto_ptr<Source> read_file( simparm::FileEntry& name );

      protected:
        Source* impl_makeSource();
      
      private:
        static File read_header
            (simparm::FileEntry& file);

      public:
        Config* clone(input::Config &newMaster) const 
            { return (new Config(*this, newMaster)); }
    };

}
}
}

#endif
