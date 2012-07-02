#include "GUILabelTable.h"
#include <boost/tokenizer.hpp>

namespace simparm {

GUILabelTable::GUILabelTable() {}

GUILabelTable& GUILabelTable::get_singleton() {
    static GUILabelTable label_table;
    return label_table;
}

const GUILabelTable::Entry& GUILabelTable::get_entry( const std::string& name ) {
    return entries[name];
}

void GUILabelTable::read_csv_file( std::istream& i ) {
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(";");
    while ( true ) {
        std::string str;
        std::getline( i, str );
        if ( ! i ) return;
        tokenizer tokens(str, sep);
        tokenizer::iterator token = tokens.begin();
        std::string name;
        Entry entry;
        if ( token != tokens.end() ) name = *token++;
        if ( token != tokens.end() ) entry.description = *token++;
        if ( token != tokens.end() ) entry.help = *token++;
        entry.helpID = name;
        entries[name] = entry;
    }

}

}
