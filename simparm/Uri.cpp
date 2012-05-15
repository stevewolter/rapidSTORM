#include "URI.hh"

namespace simparm {

URI::URI(std::string name, std::string desc) 
: Object(name,desc),
  uri("uri", "")
{
    push_back(uri);
}

URI::URI(std::string name, std::string desc, std::string uri) 
: Object(name,desc), uri("uri", uri)
{
    push_back(this->uri);
}

URI::URI(const URI& o) 
: Object(o), uri(o.uri) 
{
    push_back(uri);
}

URI::~URI() {}

}
