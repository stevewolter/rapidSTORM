#ifndef SIMPARM_URI
#define SIMPARM_URI

#include "Object.hh"

namespace simparm {

class URI : public Object {
  protected:
    virtual string getTypeDescriptor() const
        { return string("URI"); }

  public:
    URI(std::string name, std::string desc);
    URI(std::string name, std::string desc, std::string uri);
    URI(const URI&);
    virtual ~URI();
    virtual URI* clone() const
        { return new URI(*this); }

    Attribute<std::string> uri;
};

}

#endif
