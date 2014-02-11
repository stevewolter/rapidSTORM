#ifndef DSTORM_LOCALIZATION_FILE_READER_H
#define DSTORM_LOCALIZATION_FILE_READER_H

#include "image/MetaInfo.h"
#include <iostream>
#include <memory>
#include <string>
#include "input/FileInput.h"
#include <simparm/FileEntry.h>
#include "input/Source.h"
#include "engine/Image.h"
#include "localization/Traits.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/signals2/connection.hpp>

#include "localization_file/field_decl.h"
#include "localization/record.h"

namespace dStorm {
namespace localization_file {

/** This namespace provides a source that can read STM files.
 *  Standard Source semantics apply. */

namespace Reader {
    struct File {
        typedef boost::ptr_vector<Field> Fields;
        typedef input::Traits<localization::Record> Traits;

        std::string filename;
        boost::shared_ptr<std::istream> stream_store;
        std::istream& input;
        Traits traits;

        File(std::string filename, const Traits& );
        ~File();
        std::auto_ptr<Traits> getTraits() const;
        localization::Record read_next();

        int number_of_newlines();

      private:
        Fields fs;

        void read_classic(const std::string& first_line, Traits& t);
        void read_XML(const std::string& first_line, Traits& t);
    };

    class Source 
    : public input::Source<localization::Record>
    {
      public:
        Source(const File& file);

        bool GetNext(int thread, localization::Record* output) OVERRIDE;
        TraitsPtr get_traits(BaseSource::Wishes);
        Capabilities capabilities() const { return Capabilities().set( Repeatable ); }

      private:
        typedef std::list< std::vector<Localization> > TraceBuffer;

        TraceBuffer trace_buffer;
        File file;

        localization::Record read_localization(); 

        void dispatch(Messages m) { assert( ! m.any() ); }
        void attach_ui_(simparm::NodeHandle ) {}
        void set_thread_count(int num_threads) OVERRIDE {}
    };

    class Config 
    {
        simparm::Object name_object;
    public:
        Config();
        void attach_ui( simparm::NodeHandle at ) {}
    };

    class ChainLink
    : public input::FileInput< ChainLink, File >
    {
        Config config;
        friend class input::FileInput<ChainLink,File>;
        File* make_file( const std::string& ) const;
        void modify_meta_info( input::MetaInfo& info );
        void attach_ui( simparm::NodeHandle n ) { config.attach_ui(n); }
        static std::string getName() { return "STM"; }

      public:
        ChainLink();
        ~ChainLink();

        virtual input::Source<localization::Record>* makeSource();
        virtual ChainLink* clone() const { return new ChainLink(*this); }

        static std::auto_ptr<Source> read_file( simparm::FileEntry& name, const input::Traits<localization::Record>& context );

    };

}
}
}

#endif
