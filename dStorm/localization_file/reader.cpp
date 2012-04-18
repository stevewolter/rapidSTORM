#include "debug.h"

#include <dStorm/unit_interval.h>
#include "reader.h"
#include "field.h"
#include "unknown_field.h"

#include <fstream>
#include <ctype.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/units/io.hpp>

#include <boost/variant/get.hpp>
#include <dStorm/helpers/clone_ptr.hpp>
#include <dStorm/signals/InputFileNameChange.h>

namespace dStorm {
namespace localization_file {
namespace Reader {

class Source::iterator
: public boost::iterator_facade<iterator,localization::Record,std::input_iterator_tag>
{
  public:
    iterator() : file(NULL) {}
    iterator(Source& src) 
        : file(&src.file), reducer(src.reducer->clone()) { increment(); }

    void increment() { 
        trace_buffer.clear(); 
        DEBUG("Reading localization");
        current = read_localization( file->level );
        if ( ! file->input ) { file = NULL; }
        DEBUG("Read localization, file is now " << file);
    }

    localization::Record& dereference() const { return current; }
    bool equal( const iterator& o ) const { return file == o.file; }

  private:
    File* file;
    mutable localization::Record current;

    typedef std::list< std::vector<Localization> > TraceBuffer;
    TraceBuffer trace_buffer;
    boost::clone_ptr<output::TraceReducer> reducer;

    localization::Record read_localization( int level ); 

};

Source::Source( const File& file, 
                std::auto_ptr<output::TraceReducer> red )
: simparm::Object("STM_Show", "Input options"),
    file(file.filename, file.traits),
    reducer(red)
{
}

int File::number_of_newlines() {
    int lines = 0;
    while ( input.peek() == '\r'  || input.peek() == '\n' ) {
        char c;
        input.get(c);
        if ( c == '\n' ) lines++;
    }
    return lines;
}

localization::Record Source::iterator::read_localization( int level )
{
    typedef Localization::Position::Type Pos;
    static const Pos no_shift = Pos::Zero();

    static std::string missing_image_line("# No localizations in image ");

    if ( level == 0 ) {
        DEBUG("Reading at level 0");
        while (true ) {
            char peek;
            while ( true ) {
                peek = file->input.peek();
                if ( isspace(peek) ) file->input.get(); else break;
            }
            if ( peek == '#' ) {
                std::string line;
                std::getline( file->input, line );
                if ( line.find_first_of( missing_image_line ) == 0 ) {
                    std::stringstream s( line.substr(missing_image_line.length()) );
                    int n;
                    s >> n;
                    return localization::EmptyLine(n * camera::frame);
                }
            } else
                break;
        }
        DEBUG("Reading next localization");
        return file->read_next();
    } else {
        TraceBuffer::iterator my_buffer = 
            trace_buffer.insert( trace_buffer.end(), std::vector<Localization>() );

        do {
            localization::Record r = read_localization(level-1);
            if ( Localization *l = boost::get<Localization>(&r) )
                my_buffer->push_back( *l );
            else
                return r;
        } while ( file->input && file->number_of_newlines() <= level );

        localization::Record rv;
        reducer->reduce_trace_to_localization( 
            my_buffer->begin(), my_buffer->end(),
            &boost::get<Localization>(rv), no_shift );
        return rv;
    }
}

input::Source<localization::Record>::iterator Source::begin() { 
    return input::Source<localization::Record>::iterator(iterator(*this)); 
}
input::Source<localization::Record>::iterator Source::end() { 
    return input::Source<localization::Record>::iterator(iterator()); 
}

Config::Config() 
: simparm::Object("STM", "Localizations file")
{
}

void ChainLink::modify_meta_info( input::MetaInfo& info ) {
    info.accepted_basenames.push_back( std::make_pair("extension_stm", ".stm") );
    info.accepted_basenames.push_back( std::make_pair("extension_txt", ".txt") );
}

File* ChainLink::make_file( const std::string& name ) const
{
    return new File( name, File::Traits() );
}


input::Source<localization::Record>* ChainLink::makeSource() {
    return new Source(*get_file(), config.trace_reducer.make_trace_reducer());
}

File::File(std::string filename, const File::Traits& traits ) 
: filename(filename),
  stream_store( (filename == "-") ? NULL : 
                new std::ifstream(filename.c_str(), std::ios_base::in) ),
  input( (stream_store.get()) ? *stream_store : std::cin),
  traits(traits)
{
    this->traits.source_traits.clear();
    std::string line;
    std::getline(input, line);
    if ( ! input ) 
        throw std::runtime_error
            ("File does not exist or permission denied.");

    if ( line[0] == '#' )
        read_XML(line.substr(1), this->traits);
    else
        throw std::runtime_error("Localization file format not recognized");

    level = std::max<int>(number_of_newlines() - 1, 0);
}

File::~File() {}

std::auto_ptr< input::Traits<localization::Record> > File::getTraits() const {
    return std::auto_ptr<Traits>(new Traits(traits));
}

localization::Record File::read_next()
{
    localization::Record rv = Localization();
    Localization& target = boost::get<Localization>(rv);
    Fields::iterator i;
    for ( i = fs.begin(); i != fs.end(); ++i ) {
        i->parse( input, target );
    }
    if ( input && input.peek() != '\r' && input.peek() != '\n' )
        throw std::runtime_error("Expected end of line in localization file " + filename + ", " +
           "but found '" + std::string(1, input.peek()) + "'. This file seems broken.");
    return rv;
}

void File::read_XML(const std::string& line, Traits& t) {
    fs.clear();
    TiXmlDocument document;
    document.Parse( line.c_str() );
    TiXmlElement* topNode = document.RootElement();
    for (TiXmlNode* child = topNode->FirstChild(); child; child = child->NextSibling() ) {
        Field::Ptr p = Field::parse(*child, t);
        if ( p.get() != NULL )
            fs.push_back( p );
    }
    t.in_sequence = topNode->Attribute("insequence") && topNode->Attribute("insequence") == std::string("true");
}

std::auto_ptr<Source> ChainLink::read_file( simparm::FileEntry& name, const input::Traits<localization::Record>& my_context )
{
    try {
        File header = File( name(), my_context );
        output::TraceReducer::Config trc;
        Source *src = new Source(header, trc.make_trace_reducer() );
        return std::auto_ptr<Source>(src);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error( 
            "Unable to open localization file " + name() +
            std::string(": ") + e.what() );
    }
}

Source::TraitsPtr Source::get_traits(input::BaseSource::Wishes r) { 
    TraitsPtr tp( file.getTraits().release() ); 
    return tp;
}

ChainLink::ChainLink() {}
ChainLink::~ChainLink() {}

}

}
}
