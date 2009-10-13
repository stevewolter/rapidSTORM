#include <CImgBuffer/Source_impl.h>
#include "LocalizationFileReader.h"
#include <fstream>
#include <ctype.h>

namespace CImgBuffer {
template class Source<dStorm::Localization>;
}

namespace dStorm {
namespace LocalizationFileReader {

Source::Source( const STM_File& file, std::auto_ptr<TraceReducer> red )
: CImgBuffer::Source<Localization>
    (BaseSource::Pushing | 
        BaseSource::Pullable | BaseSource::Managing),
    simparm::Object("STM_Show", "Input options"),
    file(file),
    reducer(red)
{
    *(CImgBuffer::Traits<Localization>*)this
        = file.traits;
    level = std::max<int>(number_of_newlines() - 1, 0);
}

Localization* Source::fetch(int) {
    int use_trace_buffer = 0;
    read_localization( buffer, this->level, use_trace_buffer );
    if ( file.input )
        return &buffer;
    else
        return NULL;
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

void Source::read_localization(
    Localization& target, int level, int& use_buffer
)
{
    if ( level == 0 ) {
        double v[4];
        for (int i = 0; i < file.number_of_fields; i++)
            file.input >> v[i];
        for (int i = file.number_of_fields; i < 4; i++)
            v[i] = 0;
        new(&target) Localization(v[0], v[1], v[2], v[3]);
    } else {
        int my_buffer = use_buffer++;
        if ( my_buffer >= int(trace_buffer.size()) )
            trace_buffer.push_back( dStorm::Trace() );
        else
            trace_buffer[my_buffer].clear();

        while ( file.input && number_of_newlines() <= level ) {
            read_localization(buffer, level-1, use_buffer);
            trace_buffer[my_buffer].push_back( buffer );
        }
        reducer->reduce_trace_to_localization( trace_buffer[level-1],
            &target, Eigen::Vector2d(0,0) );
    }
}

STM_File Config::read_header
    (simparm::FileEntry& file)
{
    std::string line;
    std::istream& input = file.get_input_stream();
    std::getline(input, line);
    if ( ! input ) 
        throw std::runtime_error("Unable to open file " + file() );
    for (unsigned int i = 0; i < line.size(); i++) {
        char c = line[i];
        if ( ! (isdigit(c) || c == 'e' || c == 'E' || c == '+' ||
               c == '-' || c == ' ' || c == '\r' ) )
            throw std::runtime_error("Invalid STM file " + file());
    }

    STM_File header(input);
    std::stringstream fields(line);
    int fn = 0;
    do {
        double value;
        fields >> value;
        if (!fields) break;
        switch (fn) {
            case 0: header.traits.width = int(value); break;
            case 1: header.traits.height = int(value); break;
            case 2: header.traits.imageNumber = int(value); break;
            default: break;
        }
        fn++;
    } while (true);
    header.number_of_fields = fn;
    return header;
}

Config::~Config() {}

Source* Config::impl_makeSource()
{
    if ( master.inputFile() == "" )
        throw std::logic_error("No input file name set.");

    STM_File header = read_header(master.inputFile);
    
    Source *src = new Source(header, trace_reducer.make_trace_reducer() );
    src->compute_resolution( master );
    src->push_back( master.inputFile );
    return src;
}
}
}
