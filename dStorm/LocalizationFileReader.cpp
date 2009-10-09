#include <CImgBuffer/Source_impl.h>
#include "LocalizationFileReader.h"
#include <fstream>
#include <ctype.h>

namespace CImgBuffer {
template class Source<dStorm::Localization>;
}

namespace dStorm {
namespace LocalizationFileReader {

Localization* Source::fetch(int)
{
    double v[4];
    for (int i = 0; i < file.number_of_fields; i++)
        file.input >> v[i];
    for (int i = file.number_of_fields; i < 4; i++)
        v[i] = 0;
    if ( file.input )
        return new(&buffer) Localization(v[0], v[1], v[2], v[3]);
    else
        return NULL;
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
               c == '-' || c == ' ' ) )
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
    
    Source *src = new Source(header);
    src->compute_resolution( master );
    src->push_back( master.inputFile );
    return src;
}
}
}
