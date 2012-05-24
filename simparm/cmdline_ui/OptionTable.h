#ifndef SIMPARM_CMDLINE_UI_OPTION_TABLE_H
#define SIMPARM_CMDLINE_UI_OPTION_TABLE_H

#include <string>
#include <vector>
#include <stdexcept>

namespace simparm {

class BaseAttribute;

namespace cmdline_ui {

class OptionTable {
public:
    enum Type { Boolean, Value, Trigger };
private:
    struct Option {
        std::string name;
        int usage_count;
        std::vector<BaseAttribute*> attributes;
        Type type;

        Option(std::string name) : name(name), usage_count(0) {}
        void add( BaseAttribute& a, Type t ) {
            if ( usage_count > 0 && type != t )
                throw std::runtime_error("Inconsistent option type");
            else {
                type = t;
                attributes.push_back( &a );
                ++usage_count;
            }
        }
        bool operator<( const Option& o ) const { return name < o.name; }
    };
    std::vector< Option > options;
    
public:
    void add_option( std::string name, BaseAttribute& a, Type t );
    int parse( int argc, char** args );
};

}
}

#endif
