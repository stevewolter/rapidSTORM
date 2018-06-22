#include "unit_interval.h"
#include "localization_file/reader.h"
#include "localization_file/field.h"
#include "localization_file/unknown_field.h"

#include <fstream>
#include <ctype.h>

#include <boost/units/io.hpp>

#include <boost/variant/get.hpp>
#include "helpers/clone_ptr.hpp"
#include "signals/InputFileNameChange.h"

namespace dStorm {
namespace localization_file {
namespace Reader {

Source::Source( const File& file )
: file(file.filename, file.traits)
{
}

bool Source::GetNext(int thread, localization::Record* output) { 
    if (thread != 0) {
        throw std::logic_error("Localization file reading must be single-threaded.");
    }
    if (!file.input) {
        return false;
    }
    trace_buffer.clear();
    *output = read_localization();
    return bool(file.input);
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

localization::Record Source::read_localization()
{
    static std::string missing_image_line("# No localizations in image ");

    while (true ) {
        char peek;
        while ( true ) {
            peek = file.input.peek();
            if ( isspace(peek) ) file.input.get(); else break;
        }
        if ( peek == '#' ) {
            std::string line;
            std::getline( file.input, line );
            if ( line.find_first_of( missing_image_line ) == 0 ) {
                std::stringstream s( line.substr(missing_image_line.length()) );
                int n;
                s >> n;
                return localization::EmptyLine(n * camera::frame);
            }
        } else
            break;
    }
    return file.read_next();
}

Config::Config() 
: name_object("STM")
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
    return new Source(*get_file());
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
        Source *src = new Source(header);
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
