#ifndef DSTORM_LOCALIZATION_FILE_READER_H
#define DSTORM_LOCALIZATION_FILE_READER_H

#include <dStorm/ImageTraits.h>
#include <iostream>
#include <memory>
#include <string>
#include <dStorm/input/FileInput.h>
#include <simparm/FileEntry.hh>
#include <simparm/Structure.hh>
#include <dStorm/input/Source.h>
#include <dStorm/engine/Image.h>
#include <dStorm/output/TraceReducer.h>
#include <dStorm/localization/Traits.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/signals2/connection.hpp>

#include "field_decl.h"
#include <dStorm/localization/record.h>

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
        int level;
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
    : public simparm::Object, public input::Source<localization::Record>
    {
      public:
        class iterator;
        friend class iterator;

      private:
        File file;
        std::auto_ptr<output::TraceReducer> reducer;

        void dispatch(Messages m) { assert( ! m.any() ); }
        simparm::Node& node() { return *this; }

      public:
        Source(const File& file, std::auto_ptr<output::TraceReducer>);

        simparm::Node& getNode() { return *this; }
        const simparm::Node& getNode() const { return *this; }

        input::Source<localization::Record>::iterator begin();
        input::Source<localization::Record>::iterator end();
        TraitsPtr get_traits(BaseSource::Wishes);
        Capabilities capabilities() const { return Capabilities().set( Repeatable ); }
    };

    struct Config 
    : public simparm::Object
    {
        output::TraceReducer::Config trace_reducer;
        Config();
        void registerNamedEntries() { push_back( trace_reducer ); }
    };

    class ChainLink
    : public input::FileInput< ChainLink, File >
    {
        simparm::Structure<Config> config;
        friend class input::FileInput<ChainLink,File>;
        File* make_file( const std::string& ) const;
        void modify_meta_info( input::chain::MetaInfo& info );

      public:
        ChainLink();
        ~ChainLink();

        virtual input::Source<localization::Record>* makeSource();
        virtual ChainLink* clone() const { return new ChainLink(*this); }
        virtual simparm::Object& getNode() { return config; }

        static std::auto_ptr<Source> read_file( simparm::FileEntry& name, const input::Traits<localization::Record>& context );

    };

}
}
}

#endif
