#ifndef SIMPARM_GUILABELTABLE_H
#define SIMPARM_GUILABELTABLE_H

#include <boost/noncopyable.hpp>
#include <string>
#include <iosfwd>
#include <map>

namespace simparm {

class GUILabelTable : private boost::noncopyable {
public:
    static GUILabelTable& get_singleton();
    struct Entry {
        std::string description, help, helpID;
    };

    const Entry& get_entry( const std::string& name ) const;
    const std::string& get_description( const std::string& name ) const
        { return get_entry(name).description; }
    void read_csv_file( std::istream& );
private:
    GUILabelTable();
    std::map< std::string, Entry > entries;
};

}

#endif
