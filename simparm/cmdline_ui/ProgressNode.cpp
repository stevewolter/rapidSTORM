#include "ProgressNode.h"
#include <boost/lexical_cast.hpp>

namespace simparm {
namespace cmdline_ui {

void ProgressNode::add_attribute( simparm::BaseAttribute& a ) {
    if ( a.get_name() == "value" ) {
        value = &a;
        set_value();
        connections = value->notify_on_value_change( boost::bind( &ProgressNode::set_value, this ) );
    }
}

void ProgressNode::set_value() {
    std::string value_string = value->get_value();
    assert ( value_string.substr(0,4) == "set " );
    value_string = value_string.substr(4);
    double value = boost::lexical_cast<double>( value_string );

    int progressLevel = round( value * 100 );

    if (last_percentage > progressLevel) {
        std::cerr << "\r";
        last_percentage = 0;
    }
    while (last_percentage < progressLevel) {
        last_percentage++;
        if (last_percentage % 10 == 0)
            std::cerr << " " << last_percentage << " ";
        else if (last_percentage % 2 == 0)
            std::cerr << ':';

        if (last_percentage == 100)
            std::cerr << std::endl;
    }
}

}
}
