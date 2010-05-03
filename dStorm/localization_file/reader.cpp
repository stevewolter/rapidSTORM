#include "debug.h"

#include "reader.h"
#include "fields.h"
#include "known_fields.h"

#include <dStorm/input/Source_impl.h>
#include <fstream>
#include <ctype.h>

#include <dStorm/input/FileBasedMethod_impl.h>

namespace dStorm {
namespace input {
template class FileBasedMethod<Localization>;
}

namespace LocalizationFile {
namespace Reader {

Source::Source( const File& file, 
                std::auto_ptr<output::TraceReducer> red )
: simparm::Object("STM_Show", "Input options"),
  input::Source<Localization>
    (*this, BaseSource::Pushing | 
        BaseSource::Pullable | BaseSource::Managing),
    file(file),
    reducer(red),
    empty_image(NULL)
{
    static_cast<input::Traits<Localization>&>(*this)
        = file.getTraits();
    level = std::max<int>(number_of_newlines() - 1, 0);
    DEBUG("In Source constructor, resolution set is " << user_resolution.is_set());
    if ( user_resolution.is_set() )
        this->resolution = user_resolution;
}

Localization* Source::fetch(int) {
    int use_trace_buffer = 0;
    buffer.clear();
    Localization* l = buffer.allocate(1);
    read_localization( *l, this->level, use_trace_buffer );
    if ( file.input ) {
        buffer.commit(1);
        return l;
    } else {
        return NULL;
    }
}

int Source::number_of_newlines() {
    int lines = 0;
    while ( file.input.peek() == '\r'  || file.input.peek() == '\n' ) {
        char c;
        file.input.get(c);
        if ( c == '\n' ) lines++;
    }
    return lines;
}

static const char *missing_image_line = "# No localizations in image ";

void Source::read_localization(
    Localization& target, int level, int& use_buffer
)
{
    typedef Localization::Position Pos;
    static const Pos no_shift = Pos::Constant( 0 );

    if ( level == 0 ) {
      while (true ) {
        char peek;
        while ( true ) {
            peek = file.input.peek();
            if ( isspace(peek) ) file.input.get(); else break;
        }
        if ( peek == '#' ) {
            static const int len = strlen(missing_image_line);
            int i = 0;
            while ( i != len && i >= 0 )
                if ( file.input.peek() == missing_image_line[i] ) {
                    file.input.get();
                    i++;
                } else
                    i = -1;

            if ( i == len ) {
                int missing_image;
                file.input >> missing_image;
                if ( empty_image != NULL ) {
                    EmptyImageCallback::EmptyImageInfo info;
                    info.number = missing_image * cs_units::camera::frame;
                    empty_image->notice_empty_image( info );
                }
            } else {
                while ( file.input.get() != '\n' ) ;
            }
        } else
            break;
      }
      new(&target) Localization( Pos(Pos::Zero()), 0 );
      file.read_next( target );
    } else {
        int my_buffer = use_buffer++;
        if ( my_buffer >= int(trace_buffer.size()) )
            trace_buffer.push_back( dStorm::output::Trace() );
        else
            trace_buffer[my_buffer].clear();

        do {
            Localization* buffer = trace_buffer[my_buffer].allocate(1);
            read_localization(*buffer, level-1, use_buffer);
            trace_buffer[my_buffer].commit(1);
        } while ( file.input && number_of_newlines() <= level );
        reducer->reduce_trace_to_localization( trace_buffer[level-1],
            &target, no_shift );
    }
}

Config::Config(input::Config& master)
: input::FileBasedMethod<Localization>
        (master, "STM", "Localizations file",
                 "extension_stm", ".stm"),
    txt_extension("extension_txt", ".txt") 
{
    DEBUG("Beginning Config constructor");
    push_back( master.pixel_size_in_nm );
    push_back(master.firstImage);
    push_back(master.lastImage);

    master.inputFile.push_back( txt_extension );

    push_back( trace_reducer );
    DEBUG("Finished Config constructor");
}

File Config::read_header
    (simparm::FileEntry& file)
{
    try {
        std::istream& input = file.get_input_stream();
        return File(input);
    } catch (const std::exception& e) {
        throw std::runtime_error( 
            "Unable to open localization file " + file() +
            std::string(": ") + e.what() );
    }
}

File::File(std::istream& input) 
: input(input)
{
    std::string line;
    std::getline(input, line);
    if ( ! input ) 
        throw std::runtime_error
            ("File does not exist or permission denied.");

    if ( line[0] == '#' )
        read_XML(line.substr(1));
    else
        read_classic(line);

}

File::~File() {}

input::Traits<Localization> File::getTraits() const {
    input::Traits<Localization> rv;
    Fields::const_iterator i;
    for ( i = fs.begin(); i != fs.end(); ++i )
        i->getTraits( rv );
    return rv;
}

void File::read_next( Localization& target )
{
    Fields::iterator i;
    for ( i = fs.begin(); i != fs.end(); i++ ) {
        i->parse( input, target );
    }
}

void File::read_XML(const std::string& line) {
    XMLNode topNode = XMLNode::parseString( line.c_str() );
    for (int i = 0; i < topNode.nChildNode(); i++) {
        field::Interface::Ptr p = 
            field::Interface::parse
                (topNode.getChildNode(i));
        if ( p.get() != NULL )
            fs.push_back( p );
    }
}

void File::read_classic(const std::string& line) {
    for (unsigned int i = 0; i < line.size(); i++) {
        char c = line[i];
        if ( ! (isdigit(c) || c == 'e' || c == 'E' || c == '+' ||
               c == '-' || c == ' ' || c == '\r' ) )
            throw std::runtime_error("Invalid first line, only digits and spaces expected.");
    }

    std::stringstream values(line);
    int fn = 0;
    do {
        float value;
        values >> value;
        if (!values) break;
        switch (fn) {
            case 0: 
                fs.push_back( 
                    new field::XCoordinate( 
                        field::XCoordinate::Bound(
                            value * cs_units::camera::pixel) ) 
                );
                break;
            case 1:
                fs.push_back( 
                    new field::YCoordinate(
                        field::YCoordinate::Bound(
                            value * cs_units::camera::pixel ) )
                );
                break;
            case 2:
                fs.push_back(
                    new field::FrameNumber
                        ( int(value) * cs_units::camera::frame )
                );
                break;
            case 3:
                fs.push_back( new field::Amplitude() );
                break;
            default:
                fs.push_back( 
                    new field::Unknown<double>() );
                break;
        }
        fn++;
    } while (true);
}

Config::~Config() {}

Source* Config::impl_makeSource()
{
    if ( this->inputFile() == "" )
        throw std::logic_error("No input file name set.");

    File header = read_header(this->inputFile);
    
    Source *src = new Source(header, trace_reducer.make_trace_reducer() );
    src->push_back( this->inputFile );
    src->set_default_pixel_size( cs_units::camera::pixels_per_meter / (((float)master.pixel_size_in_nm()) / 1E9f) );
    return src;
}

std::auto_ptr<Source> Config::read_file( simparm::FileEntry& name )
{
    File header = read_header( name );
    output::TraceReducer::Config trc;
    Source *src = new Source(header, trc.make_trace_reducer() );
    return std::auto_ptr<Source>(src);
}

void Source::set_default_pixel_size(const File::Traits::Resolution::value_type& resolution)
{
    if ( !user_resolution.is_set() ) {
        user_resolution = resolution;
        this->resolution = resolution;
    }
}

Source::EmptyImageCallback* Source::setEmptyImageCallback( EmptyImageCallback* cb ) {
    std::swap( empty_image, cb );
    return cb;
}

}
}
}
